/*
 * OSOYOO 10.1" (800x1280) MIPI-DSI panel bring-up for the
 * Waveshare ESP32-P4-Module-DEV-KIT (ESP32-P4).
 *
 * Brings up: DSI PHY LDO, I2C "display MCU" (reset + backlight) at 0x45,
 * the ILI9881C DSI panel, and the Goodix GT911/GT9271 capacitive touch at 0x5D.
 */
#pragma once

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Panel geometry (native portrait) --- */
#define OSOYOO_LCD_H_RES        800
#define OSOYOO_LCD_V_RES        1280

/* --- Board pins (Waveshare ESP32-P4-Module-DEV-KIT, DSI FPC connector) --- */
#define OSOYOO_I2C_PORT         I2C_NUM_0
#define OSOYOO_PIN_I2C_SDA      7      /* GPIO7 on the DSI connector */
#define OSOYOO_PIN_I2C_SCL      8      /* GPIO8 on the DSI connector */
#define OSOYOO_I2C_HZ           100000 /* OSOYOO MCU is happiest at 100 kHz */

/* "display MCU" on the panel PCB (reset + PWM backlight) */
#define OSOYOO_MCU_I2C_ADDR     0x45
/* Goodix touch controller */
#define OSOYOO_TOUCH_I2C_ADDR   0x5D

/**
 * @brief Bring up the whole display stack.
 *
 * On success @p ret_panel drives an 800x1280 RGB565/RGB888 framebuffer and
 * @p ret_touch (may be NULL on failure) reports touch points.
 */
esp_err_t osoyoo_panel_init(esp_lcd_panel_handle_t *ret_panel,
                            esp_lcd_panel_io_handle_t *ret_io,
                            esp_lcd_touch_handle_t *ret_touch);

/**
 * @brief Set backlight brightness.
 * @param percent 0..100 (0 = off).
 */
esp_err_t osoyoo_panel_set_backlight(int percent);

#ifdef __cplusplus
}
#endif
