/*
 * OSOYOO 10.1" DSI + LVGL on Waveshare ESP32-P4-Module-DEV-KIT.
 *
 * Brings up the panel, starts esp_lvgl_port (display + touch), and builds the
 * SquareLine-style UI (a "Hello" label toggled by a button).
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_lvgl_port.h"
#include "lvgl.h"

#include "osoyoo_panel.h"
#include "ui/ui.h"

static const char *TAG = "app";

/* Label drawn on the toggle button (created in app_main; the SquareLine button
 * has no child label of its own). */
static lv_obj_t *s_btn_label;

/* Toggle handler: show/hide the "Hello" label based on the button's checked state.
 * Wired here (not in SquareLine's ui_events.c) so it survives every re-export. */
static void hello_toggle_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    bool on = lv_obj_has_state(btn, LV_STATE_CHECKED);
    if (on) {
        lv_obj_clear_flag(ui_Label1, LV_OBJ_FLAG_HIDDEN);   /* show "Hello" */
    } else {
        lv_obj_add_flag(ui_Label1, LV_OBJ_FLAG_HIDDEN);     /* hide "Hello" */
    }
    if (s_btn_label) {
        lv_label_set_text(s_btn_label, on ? "Hide Hello" : "Show Hello");
    }
}

void app_main(void)
{
    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_touch_handle_t touch = NULL;

    ESP_ERROR_CHECK(osoyoo_panel_init(&panel, &io, &touch));

    /* ---- LVGL port ---- */
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_cfg.task_priority = 4;
    lvgl_cfg.task_stack = 8192;
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io,
        .panel_handle = panel,
        .buffer_size = OSOYOO_LCD_H_RES * 100,
        .double_buffer = true,
        .hres = OSOYOO_LCD_H_RES,
        .vres = OSOYOO_LCD_V_RES,
        .monochrome = false,
        /* LVGL 8: color depth comes from CONFIG_LV_COLOR_DEPTH_16 (sdkconfig);
         * there is no .color_format / .swap_bytes field (those are LVGL 9 only). */
        .rotation = { .swap_xy = false, .mirror_x = false, .mirror_y = false },
        .flags = {
            .buff_dma = false,
            .buff_spiram = true,
        },
    };
    lvgl_port_display_dsi_cfg_t dsi_cfg = {
        .flags = { .avoid_tearing = false },
    };
    lv_display_t *disp = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);
    if (disp == NULL) {
        ESP_LOGE(TAG, "failed to register LVGL display");
        return;
    }

    if (touch != NULL) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = disp,
            .handle = touch,
        };
        if (lvgl_port_add_touch(&touch_cfg) == NULL) {
            ESP_LOGW(TAG, "failed to register touch with LVGL");
        }
    }

    /* ---- Build the UI (must hold the LVGL lock) ---- */
    if (lvgl_port_lock(0)) {
        ui_init();   /* SquareLine: builds ui_Screen1, ui_Button1, ui_Label1 */

        /* Make the SquareLine button a toggle and attach our handler. */
        lv_obj_add_flag(ui_Button1, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_state(ui_Button1, LV_STATE_CHECKED);             /* start: Hello shown */
        lv_obj_add_event_cb(ui_Button1, hello_toggle_cb, LV_EVENT_VALUE_CHANGED, NULL);

        /* Give the button a caption (reuse SquareLine's child label if present). */
        s_btn_label = lv_obj_get_child(ui_Button1, 0);
        if (s_btn_label == NULL) {
            s_btn_label = lv_label_create(ui_Button1);
            lv_obj_center(s_btn_label);
        }
        lv_label_set_text(s_btn_label, "Hide Hello");

        /* The button is 100x50 in the export — widen it so the caption fits. */
        lv_obj_set_width(ui_Button1, 200);

        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "UI is up. Tap the button to toggle 'Hello'.");
}
