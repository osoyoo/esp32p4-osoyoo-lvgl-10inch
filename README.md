# ESP32-P4 + OSOYOO 10.1" DSI Display with LVGL

Complete working example of driving the **OSOYOO 10.1" 800×1280 MIPI-DSI touchscreen** with an **ESP32-P4-Module-DEV-KIT** using ESP-IDF and LVGL.

## Features

✅ 2-lane MIPI-DSI display driver (ILI9881C)
✅ GT911 capacitive touch
✅ LVGL 8.3.11 UI framework
✅ SquareLine Studio compatible
✅ Tested on ESP32-P4 chip revision v1.3

## Hardware

- **Board:** ESP32-P4-Module-DEV-KIT
- **Display:** OSOYOO 10.1" 800×1280 DSI/MIPI touchscreen
- **Connection:** Single 15-pin FPC cable (carries DSI + I2C for touch/backlight)

## Quick Start

See **[SETUP_COMMAND_LINE.md](SETUP_COMMAND_LINE.md)** for complete installation and build instructions using terminal commands (no VS Code extension needed).

### TL;DR

```bash
# Install ESP-IDF v5.5.1
cd ~/esp
git clone -b v5.5.1 --depth 1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32p4
source ./export.sh

# Build and flash this project
cd /path/to/esp32p4-osoyoo-lvgl-10inch
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/cu.usbmodem* flash monitor
```

## What You'll See

"Hello World!" text with a toggle button. Tap the button to show/hide the text.

## Documentation

- **[SETUP_COMMAND_LINE.md](SETUP_COMMAND_LINE.md)** - Complete setup guide with troubleshooting
- **[main/osoyoo_panel.c](main/osoyoo_panel.c)** - Display and touch initialization
- **[main/ui/](main/ui/)** - LVGL UI files (SquareLine Studio compatible)

## Project Structure

```
esp32p4-osoyoo-lvgl-10inch/
├── main/
│   ├── main.c                  # App entry point
│   ├── osoyoo_panel.c/h        # Display + touch driver
│   ├── osoyoo_panel_init.h     # ILI9881C init sequence
│   └── ui/                     # LVGL UI (SquareLine export)
├── sdkconfig.defaults          # ESP32-P4 config (PSRAM, LVGL fonts)
└── SETUP_COMMAND_LINE.md       # Setup guide
```

## Tested Configuration

- **ESP-IDF:** v5.5.1
- **LVGL:** 8.3.11
- **Chip:** ESP32-P4 revision v1.3
- **PSRAM:** 80MHz (stable on older chip revisions)

## License

This project is provided as-is for educational and development purposes.

## Credits

Panel initialization sequence adapted from OSOYOO's Linux driver.
