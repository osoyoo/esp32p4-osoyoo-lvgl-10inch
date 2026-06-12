# OSOYOO 10.1" DSI panel on Waveshare ESP32-P4-Module-DEV-KIT (ESP-IDF + LVGL)

Drives the **OSOYOO 10.1" 800×1280 MIPI-DSI touchscreen** from a **Espressif ESP32-P4-Module-DEV-KIT**, and shows a
SquareLine-style LVGL UI: a **"Hello" label** and a **button that toggles it**.
 

---

## 1. Hardware

Plug the panel's 15-pin DSI FPC into the Espressif ESP32-P4-dev-kit's DSI connector. That single FPC
carries everything — no extra wiring:

| Signal              | Where it goes                                        |
|---------------------|------------------------------------------------------|
| DSI clock + 2 lanes | ESP32-P4 MIPI-DSI (the kit exposes **2 data lanes**) |
| Touch / backlight I2C | **SDA = GPIO7, SCL = GPIO8** (inside the DSI connector) |
| Panel reset + backlight | "display MCU" at I2C **0x45** (on the panel PCB) |
                      |

> The OSOYOO panel has its own little MCU at `0x45` that owns the LCD reset line
> and the PWM backlight (registers `0x02` = reset bits, `0x03` = backlight). There
> is **no** direct reset GPIO — this firmware toggles reset over I2C, exactly like
> the Linux driver does.

Because ESP32-P4 only has **2 DSI data lanes**, this project uses the panel's
**2-lane** init sequence (`osoyoo_10_1_inch_2lane_init`).

---

## 2. Build & flash (ESP-IDF 5.5.1 + VS Code)

1. Install the **Espressif IDF** VS Code extension and select **ESP-IDF v5.5.1**.
2. Open this `esp32p4-osoyoo-lvgl/` folder in VS Code.
3. Set the target to **esp32p4**:
   ```bash
   idf.py set-target esp32p4
   ```
   (or VS Code: *ESP-IDF: Set Espressif Device Target → esp32p4*)
4. Build / flash / monitor:
   ```bash
   idf.py build
   idf.py -p <PORT> flash monitor
   ```
   In VS Code use the 🔥 (Flash) and 🖥 (Monitor) buttons. Use the **ESP-USB-Serial/JTAG**
   port of the kit.

The first build downloads the managed components listed in
`main/idf_component.yml` (ILI9881C driver, GT911 touch, **LVGL 8.3.11**, esp_lvgl_port).

You should see "Hello" on the screen with a button under it. Tap the button to
hide/show "Hello".

---

## 3. Files

| File | Purpose |
|------|---------|
| `main/osoyoo_panel.[ch]` | DSI PHY LDO, I2C MCU power/reset/backlight, ILI9881C panel + GT911 touch bring-up |
| `main/osoyoo_panel_init.h` | ILI9881C init sequence, auto-converted from the OSOYOO Linux driver |
| `main/main.c` | Starts esp_lvgl_port (display + touch), builds the UI |
| `main/ui/ui.[ch]`, `ui/ui_events.c` | SquareLine-shaped UI (screen, button, "Hello" label, toggle handler) |
| `sdkconfig.defaults` | esp32p4, 16 MB flash, PSRAM, LVGL fonts/color depth |
| `partitions.csv` | 3 MB app partition |

---

## 4. Using SquareLine Studio for the UI

The `main/ui/` files deliberately mirror what SquareLine Studio exports, so you
can replace them with your own design:

1. In SquareLine: **Create project → Desktop / generic, LVGL 8.3,
   resolution 800 × 1280, color depth 16-bit**.
2. Add a **Button** and a **Label** (set its text to `Hello`).
3. Select the button → make it **Checkable** (toggle). Add an **Event**:
   *Trigger = VALUE_CHANGED → Add → Call function → name it `Button1`*.
   In the label, leave a known name (e.g. `LabelHello`).
4. **Export → Export UI Files**, set the export path to this project's `main/ui/`.
5. SquareLine generates `ui.c/ui.h`, `ui_events.c`, `ui_helpers.*`, and
   `screens/…`. Add any new `.c` files to `main/CMakeLists.txt` `SRCS`, then in the
   generated `ui_events.c` `Button1()` put the same show/hide logic shown in this
   project's `ui_events.c`.
6. `main.c` already calls `ui_init()` under the LVGL lock — no change needed.

