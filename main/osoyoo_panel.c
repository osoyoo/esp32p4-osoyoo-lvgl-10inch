#include "osoyoo_panel.h"
#include "osoyoo_panel_init.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"

#include "driver/i2c_master.h"
#include "esp_ldo_regulator.h"

#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9881c.h"

#include "esp_lcd_touch_gt911.h"

static const char *TAG = "osoyoo_panel";

/* --- "display MCU" register map (from OSOYOO osoyoo-panel-regulator.c) --- */
#define MCU_REG_ID          0x01
#define MCU_REG_POWERON     0x02   /* bit0 = LCD reset, bit1 = touch reset/power */
#define MCU_REG_PWM         0x03   /* bit7 = backlight enable, bits[4:0] = level  */
#define MCU_LCD_RESET_BIT   (1 << 0)
#define MCU_CTP_RESET_BIT   (1 << 1)
#define MCU_PWM_BL_ENABLE   (1 << 7)
#define MCU_PWM_MAX         0x1F

/* MIPI-DSI PHY needs a dedicated 2.5 V rail from internal LDO channel 3 */
#define DSI_LDO_CHAN_ID     3
#define DSI_LDO_VOLTAGE_MV  2500

static i2c_master_bus_handle_t s_i2c_bus;
static i2c_master_dev_handle_t s_mcu_dev;
static esp_ldo_channel_handle_t s_ldo_phy;

static esp_err_t mcu_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_mcu_dev, buf, sizeof(buf), 100 /*ms*/);
}

esp_err_t osoyoo_panel_set_backlight(int percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    uint8_t level = (uint8_t)((percent * MCU_PWM_MAX) / 100);
    if (percent > 0 && level == 0) level = 1;          /* keep it visible */
    uint8_t val = level ? (MCU_PWM_BL_ENABLE | level) : 0;
    return mcu_write_reg(MCU_REG_PWM, val);
}

/* I2C bus + display MCU power/reset sequence */
static esp_err_t bringup_i2c_and_power(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = OSOYOO_I2C_PORT,
        .sda_io_num = OSOYOO_PIN_I2C_SDA,
        .scl_io_num = OSOYOO_PIN_I2C_SCL,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_i2c_bus), TAG, "i2c bus");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OSOYOO_MCU_I2C_ADDR,
        .scl_speed_hz = OSOYOO_I2C_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_i2c_bus, &dev_cfg, &s_mcu_dev),
                        TAG, "mcu dev");

    /* Probe the MCU so we fail early with a clear message if the FPC is loose */
    esp_err_t probe = i2c_master_probe(s_i2c_bus, OSOYOO_MCU_I2C_ADDR, 100);
    if (probe != ESP_OK) {
        ESP_LOGE(TAG, "display MCU 0x%02X not found on I2C (check the DSI FPC cable / pins 7,8)",
                 OSOYOO_MCU_I2C_ADDR);
        return probe;
    }

    /* Replicate the Linux driver's reset sequence, driven over I2C:
     *   hold LCD in reset, 60 ms, release LCD + touch, settle. */
    ESP_RETURN_ON_ERROR(mcu_write_reg(MCU_REG_POWERON, 0x00), TAG, "reset assert");
    vTaskDelay(pdMS_TO_TICKS(60));
    ESP_RETURN_ON_ERROR(mcu_write_reg(MCU_REG_POWERON,
                        MCU_LCD_RESET_BIT | MCU_CTP_RESET_BIT), TAG, "reset release");
    vTaskDelay(pdMS_TO_TICKS(120));   /* panel + Goodix power-up settle */
    return ESP_OK;
}

static esp_err_t bringup_dsi_panel(esp_lcd_panel_handle_t *ret_panel,
                                   esp_lcd_panel_io_handle_t *ret_io)
{
    /* 1. DSI PHY power */
    esp_ldo_channel_config_t ldo_cfg = {
        .chan_id = DSI_LDO_CHAN_ID,
        .voltage_mv = DSI_LDO_VOLTAGE_MV,
    };
    ESP_RETURN_ON_ERROR(esp_ldo_acquire_channel(&ldo_cfg, &s_ldo_phy), TAG, "ldo");

    /* 2. DSI bus (2 data lanes) */
    esp_lcd_dsi_bus_handle_t dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = 2,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = 1000,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &dsi_bus), TAG, "dsi bus");

    /* 3. Control channel (DBI) for sending init commands */
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_dbi(dsi_bus, &dbi_config, &io), TAG, "dbi io");

    /* 4. DPI (pixel stream) config — OSOYOO 10.1" timings @ ~60 Hz */
    esp_lcd_dpi_panel_config_t dpi_config = {
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = 75,
        .virtual_channel = 0,
        .pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565,
        .num_fbs = 1,
        .video_timing = {
            .h_size = OSOYOO_LCD_H_RES,
            .v_size = OSOYOO_LCD_V_RES,
            .hsync_pulse_width = 20,
            .hsync_back_porch = 60,
            .hsync_front_porch = 60,
            .vsync_pulse_width = 6,
            .vsync_back_porch = 22,
            .vsync_front_porch = 16,
        },
        .flags.use_dma2d = true,
    };

    /* 5. ILI9881C vendor driver with the OSOYOO init sequence */
    ili9881c_vendor_config_t vendor_config = {
        .init_cmds = osoyoo_10_1_inch_2lane_init,
        .init_cmds_size = sizeof(osoyoo_10_1_inch_2lane_init) / sizeof(ili9881c_lcd_init_cmd_t),
        .mipi_config = {
            .dsi_bus = dsi_bus,
            .dpi_config = &dpi_config,
            .lane_num = 2,
        },
    };
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,                 /* reset is done over I2C, not a GPIO */
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,                 /* RGB565 */
        .vendor_config = &vendor_config,
    };
    esp_lcd_panel_handle_t panel = NULL;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_ili9881c(io, &panel_config, &panel), TAG, "ili9881c");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel), TAG, "reset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel), TAG, "init");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel, true), TAG, "disp on");

    *ret_panel = panel;
    *ret_io = io;
    return ESP_OK;
}

static esp_err_t bringup_touch(esp_lcd_touch_handle_t *ret_touch)
{
    esp_lcd_panel_io_handle_t tp_io = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_cfg = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_cfg.scl_speed_hz = OSOYOO_I2C_HZ;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(s_i2c_bus, &tp_io_cfg, &tp_io), TAG, "tp io");

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = OSOYOO_LCD_H_RES,
        .y_max = OSOYOO_LCD_V_RES,
        .rst_gpio_num = -1,   /* touch reset is handled by the display MCU */
        .int_gpio_num = -1,
        .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
    };
    return esp_lcd_touch_new_i2c_gt911(tp_io, &tp_cfg, ret_touch);
}

esp_err_t osoyoo_panel_init(esp_lcd_panel_handle_t *ret_panel,
                            esp_lcd_panel_io_handle_t *ret_io,
                            esp_lcd_touch_handle_t *ret_touch)
{
    ESP_RETURN_ON_ERROR(bringup_i2c_and_power(), TAG, "i2c/power");
    ESP_RETURN_ON_ERROR(bringup_dsi_panel(ret_panel, ret_io), TAG, "dsi");

    /* Touch is best-effort: the display still works without it. */
    if (ret_touch) {
        *ret_touch = NULL;
        esp_err_t terr = bringup_touch(ret_touch);
        if (terr != ESP_OK) {
            ESP_LOGW(TAG, "touch init failed (%s) — continuing without touch",
                     esp_err_to_name(terr));
            *ret_touch = NULL;
        }
    }

    ESP_RETURN_ON_ERROR(osoyoo_panel_set_backlight(100), TAG, "backlight");
    ESP_LOGI(TAG, "OSOYOO 10.1\" panel ready (%dx%d)", OSOYOO_LCD_H_RES, OSOYOO_LCD_V_RES);
    return ESP_OK;
}
