# Complete Tutorial — OSOYOO 10.1" DSI + LVGL on Espressif ESP32-P4-DEV-Kit

From-scratch guide: install **SquareLine Studio**, **VS Code**, the **ESP-IDF
extension + ESP-IDF 5.5.1**, design a UI in SquareLine, export the **LVGL** files,
combine them with the ESP-IDF C code, then **build & flash** to the board.

Covers **Windows** and **macOS**. Where steps differ, look for the
🪟 **Windows** and  **macOS** markers.

**Hardware this targets**
- Board: **Espressif ESP32-P4-Module-DEV-KIT** (2-lane MIPI-DSI, GPIO7=SDA, GPIO8=SCL)
- Panel: **OSOYOO 10.1" 800×1280** portrait, **ILI9881C** controller, **GT9271** touch
- The panel's reset + backlight are driven over I²C by an on-panel MCU at address `0x45`

---

## Table of contents
1. [Install SquareLine Studio](#1-install-squareline-studio)
2. [Install VS Code](#2-install-vs-code)
3. [Install the ESP-IDF extension + ESP-IDF 5.5.1](#3-install-the-esp-idf-extension--esp-idf-551)
4. [Get the project](#4-get-the-project)
5. [Design the UI in SquareLine Studio](#5-design-the-ui-in-squareline-studio)
6. [Export the LVGL files](#6-export-the-lvgl-files)
7. [Combine the LVGL files with the ESP-IDF C code](#7-combine-the-lvgl-files-with-the-esp-idf-c-code)
8. [Build, flash, and monitor](#8-build-flash-and-monitor)
9. [How the firmware fits together](#9-how-the-firmware-fits-together)
10. [Troubleshooting (every issue we hit)](#10-troubleshooting-every-issue-we-hit)

---

## 1. Install SquareLine Studio

1. Go to **https://squareline.io** → Download → pick **Windows** or **macOS**.
2. Install and launch it. Create a free account and log in (the free tier is enough).
3. Note the **LVGL version** your SquareLine supports (Help/About or the New-Project
   dialog). Many builds ship **LVGL 8.3.11** — that's what this project targets.

> The LVGL version in SquareLine **must match** the LVGL version in the firmware.
> This project uses **LVGL 8.3.11**. If your SquareLine offers LVGL 9, see the note
> in §7.

---

## 2. Install VS Code

- 🪟 **Windows:** download from **https://code.visualstudio.com**, run the installer,
  accept defaults (tick "Add to PATH").
-  **macOS:** download the `.zip`, unzip, drag **Visual Studio Code.app** to
  `/Applications`. Launch it once.

---

## 3. Install the ESP-IDF extension + ESP-IDF 5.5.1

### 3a. Install the extension
1. In VS Code open **Extensions** (`Ctrl+Shift+X` / `Cmd+Shift+X`).
2. Search **`Espressif IDF`**, install the one by **Espressif Systems** (red logo).
3. Reload VS Code if asked.

To check it loaded: `Ctrl+Shift+P` / `Cmd+Shift+P` → type `ESP-IDF` and you should see
many `ESP-IDF: …` commands. (If the Command Palette shows nothing, make sure the
search box starts with `>` — that's command mode.)

### 3b. Install the ESP-IDF framework

🪟 **Windows** (the easy path — Windows bundles its own Python):
1. `Ctrl+Shift+P` → **ESP-IDF: Configure ESP-IDF Extension** (or use the Welcome page).
2. Choose **EXPRESS**.
3. **Download server:** choose **Espressif** if GitHub is slow in your region,
   otherwise **GitHub**.
4. **ESP-IDF version:** **v5.5.1**.
5. Leave install paths default. Click **Install** and wait (downloads toolchain +
   Python; several minutes). Finish = *"All settings have been configured."*

 **macOS** (one extra prerequisite — the system Python is too old):

macOS ships **Python 3.9**, but the installer needs **Python 3.10–3.13**. Install a
compatible Python first with [Homebrew](https://brew.sh):
```bash
brew install python@3.12
```
Then try the GUI installer (`ESP-IDF: Configure ESP-IDF Extension` → Express →
v5.5.1). **If the GUI installer can't find your Homebrew Python** (a common macOS
issue — GUI apps don't see Homebrew's path), use the reliable **command-line
install** instead:

```bash
# make a python3 that points at 3.12, for the installer
mkdir -p ~/esp/.pybin
ln -sf /opt/homebrew/bin/python3.12 ~/esp/.pybin/python3
export PATH="$HOME/esp/.pybin:$PATH"
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"   # use Espressif mirror (faster)

# clone ESP-IDF 5.5.1 and install the ESP32-P4 toolchain
mkdir -p ~/esp && cd ~/esp
git clone -b v5.5.1 --depth 1 --recursive --shallow-submodules https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32p4
```
Then tell the VS Code extension to use it: `Cmd+Shift+P` → **ESP-IDF: Configure
ESP-IDF Extension → Use Existing Setup** → point at `~/esp/esp-idf` (tools:
`~/.espressif`).

> Apple-Silicon Homebrew lives in `/opt/homebrew`. On Intel Macs it's
> `/usr/local`, so use `/usr/local/bin/python3.12` in the commands above.

To verify (in an **ESP-IDF terminal**, see §8): `idf.py --version` → `ESP-IDF v5.5.1`.

---

## 4. Get the project

This repo's `esp32p4-osoyoo-lvgl/` folder is the ready-to-build ESP-IDF project. It
already contains the working **panel driver**, **touch**, **LVGL port**, build
config, and a SquareLine-style **UI** you can replace.

Open it in VS Code: **File → Open Folder…** → select **`esp32p4-osoyoo-lvgl`**
(the folder that has the top-level `CMakeLists.txt` — *not* its parent, *not* `main`).

The Explorer should show at the top level: `CMakeLists.txt`, `sdkconfig.defaults`,
`partitions.csv`, `main/`.

---

## 5. Design the UI in SquareLine Studio

### 5.1 Create the project — **get these settings right**
**Create → Project**, then:
- **Board template:** pick **Desktop → "VS Code with SDL"** (it lets you set a custom
  resolution and even preview on PC). **Do NOT pick "ESP-BOX"** — it locks you to
  320×240.
- **LVGL version:** **8.3** (must match `lvgl/lvgl ~8.3.11` in the firmware).
- **Resolution:** **Width 800, Height 1280**, **Orientation: Portrait**.
- **Color depth:** **16-bit** ← important; the panel pipeline is RGB565. If you leave
  it at 32-bit, the export adds a `#error` that won't compile (see §10).

If the canvas isn't 800×1280, fix it in **Project Settings → Display → Resolution**.

### 5.2 Build the screen
1. **Label:** drag a **Label**, rename it (Inspector, top field) to **`Label1`** (or
   anything — just remember the name), set **Text** = `Hello World!`, pick a font size
   (e.g. Montserrat 20). Align = Center.
2. **Button:** drag a **Button**, rename it **`Button1`**. Place it below the label.
   - Optional: add a child Label to the button and set its text.
3. (Optional) make the button a toggle in SquareLine: select it → **Flags** →
   **CHECKABLE**. *In this project the toggle is wired in C (`main.c`), so this is
   optional — see §7.*

> 800×1280 is tall and narrow. Use **Align = Center** with a Y offset rather than
> absolute pixel positions so it looks right on the real panel.

### 5.3 (Optional) preview on PC
If you chose "VS Code with SDL", **Export → Create Template Project** gives you a
desktop simulator you can run on your computer to preview the UI. This is separate
from the files you put on the ESP32 (next step).

---

## 6. Export the LVGL files

1. **File → Project Settings → UI Files Export Path** → set it to the project's
   **`main/ui/`** folder.
   - **Make sure the export path is `main/ui/` exactly.** If you point it at the
     project root, SquareLine's own `CMakeLists.txt` will overwrite the project's
     top-level `CMakeLists.txt` and break the build (see §10).
2. **Export → Export UI Files.**

This writes into `main/ui/`: `ui.c`, `ui.h`, `ui_events.c/.h`, `ui_helpers.c/.h`,
`screens/ui_Screen1.c/.h`, `components/ui_comp_hook.c`, plus a few non-source files
(`CMakeLists.txt`, `filelist.txt`, `project.info`) which ESP-IDF ignores.

---

## 7. Combine the LVGL files with the ESP-IDF C code

SquareLine exports plain LVGL C. Four things connect it to ESP-IDF. *(The shipped
project already has all four done; you only need to redo the ones marked "every
export" after re-exporting.)*

### 7.1 List the exported sources in the build — **every export**
Open **`main/CMakeLists.txt`** and ensure **every** `.c` in `main/ui/` is in `SRCS`:
```cmake
idf_component_register(
    SRCS
        "main.c"
        "osoyoo_panel.c"
        "ui/ui.c"
        "ui/ui_events.c"
        "ui/ui_helpers.c"
        "ui/screens/ui_Screen1.c"
        "ui/components/ui_comp_hook.c"
    INCLUDE_DIRS "." "ui" "ui/screens" "ui/components"
)
```
A missing file shows up at link time as `undefined reference to ui_Screen1_screen_init`.
Tip: run `ls main/ui` (macOS) / `dir main\ui` (Windows) and match the list.

### 7.2 Fix the LVGL include — **every export**
SquareLine's `ui.h` starts with `#include "lvgl/lvgl.h"`, which doesn't resolve in
ESP-IDF. Change it to:
```c
#include "lvgl.h"
```

### 7.3 Match the color depth — **once, set in SquareLine**
The export contains:
```c
#if LV_COLOR_DEPTH != 16
    #error "..."
#endif
```
If you set **16-bit** in SquareLine (§5.1) this passes. If you forgot and it says
`!= 32`, either set 16-bit in SquareLine and re-export, or change the `32` to `16`
in `ui.c`. The firmware renders RGB565, so **16-bit is required**.

### 7.4 Enable the fonts you used — **once, in sdkconfig**
LVGL fonts are enabled via `sdkconfig`. The shipped `sdkconfig.defaults` enables
Montserrat **20, 24, 48**. If your UI uses a different size, add it, e.g.:
```
CONFIG_LV_FONT_MONTSERRAT_16=y
```
A missing font is a build error like `'lv_font_montserrat_20' undeclared`.

### 7.5 Hook the UI (and the toggle) in `main.c`
`main.c` already calls `ui_init()` under the LVGL lock and wires the
**show/hide-Hello toggle** there (so it survives re-exports — SquareLine overwrites
`ui_events.c`):
```c
if (lvgl_port_lock(0)) {
    ui_init();                                   // SquareLine builds the screen
    lv_obj_add_flag(ui_Button1, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_state(ui_Button1, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_Button1, hello_toggle_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lvgl_port_unlock();
}
```
If you renamed your label/button in SquareLine, update the names in `main.c`
(`ui_Label1`, `ui_Button1`) to match.

---

## 8. Build, flash, and monitor

### 8.1 Connect the board
Plug the dev kit's **USB** port (the USB-Serial/JTAG one) into your computer.
Make sure the **OSOYOO panel's DSI FPC** is firmly seated in the DSI connector.

Find the port:
- 🪟 **Windows:** open **Device Manager → Ports (COM & LPT)**. Look for
  **"USB JTAG/serial debug unit (COMx)"** or **"USB Serial Device (COMx)"**. Note the
  `COMx`. (Windows 10/11 needs no driver for the native USB-JTAG.)
-  **macOS:** run `ls /dev/cu.*` — it's the **`/dev/cu.usbmodemXXXXXXXX`** entry
  (ignore `cu.debug-console` / Bluetooth).

### 8.2 Easiest: the VS Code buttons (both OSes)
With the `esp32p4-osoyoo-lvgl` folder open:
1. `Ctrl/Cmd+Shift+P` → **ESP-IDF: Set Espressif Device Target** → **esp32p4**
   (first time only).
2. `Ctrl/Cmd+Shift+P` → **ESP-IDF: Select Port to Use** → your COM/usbmodem port.
3. In the **bottom blue status bar**: 🛠 **Build**, then 🔥 **Flash**, then 🖥
   **Monitor** — or the combined **Build, Flash and Monitor** (flame+screen) icon.

The first build downloads the managed components (ILI9881C, GT911, LVGL, esp_lvgl_port)
and compiles all of LVGL — that takes several minutes. Later builds are fast.

### 8.3 Or the command line (ESP-IDF terminal)
Open an ESP-IDF terminal so `idf.py` is on PATH:
- 🪟 **Windows:** Start Menu → **"ESP-IDF X.X CMD"** (or **PowerShell**), `cd` to the
  project. Or VS Code Command Palette → **ESP-IDF: Open ESP-IDF Terminal**.
-  **macOS:** VS Code Command Palette → **ESP-IDF: Open ESP-IDF Terminal**, or in a
  plain terminal:
  ```bash
  export PATH="$HOME/esp/.pybin:$PATH"      # only needed if you used the CLI install
  . ~/esp/esp-idf/export.sh
  ```
Then:
```bash
idf.py set-target esp32p4        # first time only
idf.py build
idf.py -p <PORT> flash monitor   # <PORT> = COMx (Win) or /dev/cu.usbmodemXXXX (mac)
```
Exit the monitor with `Ctrl+]`.

### 8.4 Success looks like
On the serial monitor:
```
ili9881c: ID1: 0x98, ID2: 0x81, ID3: 0x5c
GT911: TouchPad_ID:0x39,0x32,0x37
osoyoo_panel: OSOYOO 10.1" panel ready (800x1280)
app: UI is up. Tap the button to toggle 'Hello'.
```
On the screen: **"Hello World!"** with a button. Tap it to hide/show the text.

---

## 9. How the firmware fits together

```
main.c
 └─ osoyoo_panel_init()                 (main/osoyoo_panel.c)
     ├─ I²C bus on GPIO7/8  → MCU 0x45  (LCD reset + backlight)
     ├─ DSI PHY LDO (2.5V), DSI bus (2 lanes), DBI control channel
     ├─ esp_lcd_new_panel_ili9881c()    + OSOYOO init (main/osoyoo_panel_init.h)
     ├─ DPI panel (800×1280 RGB565 timings)
     └─ GT911/GT9271 touch on the same I²C bus
 └─ esp_lvgl_port (display + touch) → LVGL
 └─ ui_init()                           (main/ui/*  — your SquareLine export)
```
You normally only edit the **SquareLine UI** and the **toggle wiring in `main.c`**.
The panel bring-up in `osoyoo_panel.c` / `osoyoo_panel_init.h` is done and working.

---

## 10. Troubleshooting (every issue we hit)

| Symptom | Cause & fix |
|---|---|
| **`idf.py: command not found`** | The ESP-IDF env isn't loaded. Use an **ESP-IDF terminal** (§8.3), or run the extension's buttons. |
| **macOS: "Python >= 3.10 < 3.14 required"** in the installer | System Python is 3.9. `brew install python@3.12`, then use the **CLI install** in §3b (the GUI installer often can't see Homebrew's Python). |
| **macOS: `python3 --version` still 3.9 after brew** | Homebrew's `python@3.12` only provides `python3.12`, not a bare `python3`. Use the `~/esp/.pybin/python3` symlink trick in §3b. |
| **Top-level `CMakeLists.txt` got overwritten** by SquareLine | Your export path was the project root. Restore the project's top-level `CMakeLists.txt` (it should contain `project(osoyoo_dsi_lvgl)`), and set the export path to **`main/ui/`**. |
| **`undefined reference to ui_Screen1_screen_init`** (link error) | A SquareLine `.c` isn't in `main/CMakeLists.txt` `SRCS` (§7.1). |
| **`'lv_font_montserrat_20' undeclared`** | Enable that font in `sdkconfig.defaults` (§7.4), then rebuild. |
| **`#error LV_COLOR_DEPTH should be 32bit`** | SquareLine project was 32-bit. Set **16-bit** in SquareLine and re-export, or edit the guard to `16` (§7.3). |
| **`fatal error: lvgl/lvgl.h`** | Change `ui.h`'s include to `#include "lvgl.h"` (§7.2). |
| **Boot log spams `lcd.dsi.dpi: can't fetch data ... underrun happens`; screen flickers/garbled** | PSRAM too slow. The boot log showed `esp_psram: Speed: 20MHz`. On ESP32-P4, **200 MHz PSRAM needs both** `CONFIG_IDF_EXPERIMENTAL_FEATURES=y` **and** `CONFIG_SPIRAM_SPEED_200M=y` (set together, else it silently drops to 20 MHz). We also set `CONFIG_CACHE_L2_CACHE_256KB=y` + `CONFIG_CACHE_L2_CACHE_LINE_128B=y`. All already in `sdkconfig.defaults`. |
| **Backlight on but screen blank (no image)** | The ILI9881C init must end **back on page 0** so the display-on command lands on the real register. The init array ends with `{0xFF,{0x98,0x81,0x00},3,0}` (page 0) + tear-on — already in `main/osoyoo_panel_init.h`. |
| **`display MCU 0x45 not found on I2C`** | DSI FPC not fully seated; re-seat so the GPIO7/GPIO8 contacts mate. |
| **Colors look byte-swapped** | `idf.py menuconfig` → Component config → LVGL → Color settings → enable **Swap the 2 bytes of RGB565** (`CONFIG_LV_COLOR_16_SWAP=y`). |
| **`esp_lcd_panel_swap_xy ... not supported`** (red E line) | Harmless — esp_lvgl_port probing rotation on a portrait-native panel. Ignore. |
| **Changed `sdkconfig.defaults` but nothing changed** | An existing `sdkconfig` overrides defaults. Delete `sdkconfig` (or `idf.py set-target esp32p4` again) so it regenerates from defaults. |

### After every SquareLine re-export — quick checklist
1. `ui.h`: `lvgl/lvgl.h` → `lvgl.h` (§7.2)
2. `main/CMakeLists.txt`: every `ui/*.c` in `SRCS` (§7.1)
3. Object names in `main.c` still match (`ui_Label1`, `ui_Button1`)
4. `idf.py build` → 🔥 flash

(Setting **16-bit color** in SquareLine once removes the §7.3 step permanently.)
