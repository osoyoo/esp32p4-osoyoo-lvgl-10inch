# ESP-IDF Command Line Setup Guide for ESP32-P4

Complete guide to set up ESP-IDF and build/flash ESP32-P4 projects using **terminal commands only** (no VS Code extension needed).

---

## Table of Contents
1. [macOS Setup](#macos-setup)
2. [Windows Setup](#windows-setup)
3. [Building and Flashing](#building-and-flashing)
4. [Common Commands](#common-commands)
5. [Troubleshooting](#troubleshooting)

---

## macOS Setup

### Prerequisites

1. **Check if Xcode Command Line Tools are installed**:
   ```bash
   xcode-select -p
   ```

   If you see `/Library/Developer/CommandLineTools`, you're good! Skip to step 2.

   If not installed, run:
   ```bash
   xcode-select --install
   ```
   Click **Install** when the popup appears.

2. **Install Homebrew** (if not already installed):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **Install required tools**:
   ```bash
   brew install cmake ninja dfu-util python@3.12
   ```

   **Important:** Use Python 3.12 specifically - newer versions (3.14+) may have SSL certificate issues.

### Install ESP-IDF v5.5.1

1. **Create ESP directory and Python symlink**:
   ```bash
   mkdir -p ~/esp
   cd ~/esp

   # Create Python symlink for ESP-IDF (ensures it uses Python 3.12)
   mkdir -p ~/esp/.pybin
   ln -sf /usr/local/bin/python3.12 ~/esp/.pybin/python3
   export PATH="$HOME/esp/.pybin:$PATH"
   ```

2. **Clone ESP-IDF v5.5.1**:
   ```bash
   git clone -b v5.5.1 --depth 1 --recursive --shallow-submodules https://github.com/espressif/esp-idf.git
   cd esp-idf
   ```

3. **Install ESP32-P4 toolchain** (use Espressif mirror to avoid SSL issues):
   ```bash
   # Use Espressif's download mirror (more reliable than GitHub)
   export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"

   # Install toolchain
   ./install.sh esp32p4
   ```

   This takes 5-10 minutes. It downloads compilers, debuggers, and Python packages.

   **If you see SSL certificate errors**, the mirror setting above should fix it. If it still fails, see [Troubleshooting](#troubleshooting).

4. **Set up environment** (run this in every new terminal):
   ```bash
   cd ~/esp/esp-idf
   source ./export.sh
   ```

5. **Verify installation**:
   ```bash
   idf.py --version
   ```
   Should output: `ESP-IDF v5.5.1`

### Make it permanent (optional)

Add to your `~/.zshrc` or `~/.bashrc`:
```bash
# ESP-IDF
alias get_idf='. ~/esp/esp-idf/export.sh'
```

Then run `get_idf` in new terminals instead of the full source command.

---

## Windows Setup

### Prerequisites

1. **Download and install**:
   - Git: https://git-scm.com/download/win
   - Python 3.12: https://www.python.org/downloads/
     - ✓ Check "Add Python to PATH" during installation

2. **Install CMake and Ninja**:
   - Download installer from: https://cmake.org/download/
   - Ninja: https://github.com/ninja-build/ninja/releases (extract to `C:\ninja` and add to PATH)

### Install ESP-IDF v5.5.1

1. **Open Command Prompt or PowerShell**

2. **Create ESP directory**:
   ```cmd
   mkdir %USERPROFILE%\esp
   cd %USERPROFILE%\esp
   ```

3. **Clone ESP-IDF v5.5.1**:
   ```cmd
   git clone -b v5.5.1 --depth 1 --recursive --shallow-submodules https://github.com/espressif/esp-idf.git
   ```

4. **Install ESP32-P4 toolchain**:
   ```cmd
   cd %USERPROFILE%\esp\esp-idf
   install.bat esp32p4
   ```

5. **Set up environment** (run in every new terminal):
   ```cmd
   cd %USERPROFILE%\esp\esp-idf
   export.bat
   ```

6. **Verify installation**:
   ```cmd
   idf.py --version
   ```

---

## Building and Flashing

### First Time Setup for a Project

1. **Navigate to your project**:
   ```bash
   cd /path/to/esp32p4-osoyoo-lvgl-10inch
   ```

2. **Set the target to ESP32-P4**:
   ```bash
   idf.py set-target esp32p4
   ```

   This creates `sdkconfig` from `sdkconfig.defaults` and configures for ESP32-P4.

### Build the Project

```bash
idf.py build
```

First build takes 5-10 minutes (downloads components, compiles LVGL). Later builds are fast (~30 seconds).

### Find Your Board's Serial Port

**macOS:**
```bash
ls /dev/cu.*
```
Look for `/dev/cu.usbmodem*` (e.g., `/dev/cu.usbmodem14201`)

**Windows:**
Check Device Manager → Ports (COM & LPT) → Look for COM port (e.g., `COM3`)

### Flash the Firmware

**macOS:**
```bash
idf.py -p /dev/cu.usbmodem14201 flash
```

**Windows:**
```cmd
idf.py -p COM3 flash
```

### Monitor Serial Output

**macOS:**
```bash
idf.py -p /dev/cu.usbmodem14201 monitor
```

**Windows:**
```cmd
idf.py -p COM3 monitor
```

**Exit monitor:** Press `Ctrl + ]`

### Flash + Monitor in One Command

```bash
idf.py -p PORT flash monitor
```

Replace `PORT` with your actual port.

---

## Common Commands

### Clean Build (when things go wrong)

```bash
# Full clean - deletes everything
idf.py fullclean

# Then rebuild
idf.py build
```

### Configuration Menu

```bash
idf.py menuconfig
```

Navigate with arrow keys, press `S` to save, `Q` to quit.

### Erase Flash (complete reset)

```bash
idf.py -p PORT erase-flash
```

### Change Serial Port Speed

```bash
idf.py -p PORT -b 115200 monitor
```

### Build Only Bootloader

```bash
idf.py bootloader
```

### Build Only Application

```bash
idf.py app
```

### Check Build Size

```bash
idf.py size
```

---

## Troubleshooting

### Issue: `idf.py: command not found`

**Solution:** You didn't source the ESP-IDF environment.

**macOS/Linux:**
```bash
cd ~/esp/esp-idf
source ./export.sh
```

**Windows:**
```cmd
cd %USERPROFILE%\esp\esp-idf
export.bat
```

---

### Issue: SSL Certificate Error During Installation

**Symptom:** `[SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed: unable to get local issuer certificate`

**Solution:** Use Espressif's download mirror instead of GitHub:

```bash
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
./install.sh esp32p4
```

**Alternative Solution** (if mirror doesn't work):

1. Make sure you're using Python 3.12 (not 3.14 or newer):
   ```bash
   python3 --version  # Should show 3.12.x
   ```

2. If using wrong Python version, create symlink:
   ```bash
   mkdir -p ~/esp/.pybin
   ln -sf /usr/local/bin/python3.12 ~/esp/.pybin/python3
   export PATH="$HOME/esp/.pybin:$PATH"
   ```

3. Try installation again with mirror:
   ```bash
   export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
   ./install.sh esp32p4
   ```

---

### Issue: `Chip revision v1.3 but bootloader requires v3.1+`

**Solution:** Your ESP32-P4 chip is an older revision. Edit `sdkconfig.defaults`:

```bash
nano sdkconfig.defaults
```

Add this line at the top:
```
CONFIG_ESP32P4_REV_MIN_0=y
```

Then:
```bash
rm -rf build sdkconfig
idf.py build
```

---

### Issue: Watchdog timer reset loop / Boot crash

**Symptom:** Constant reboots showing `rst:0x10 (CHIP_LP_WDT_RESET)`

**Solution:** PSRAM settings too aggressive for your chip. Edit `sdkconfig.defaults`:

Change from:
```
CONFIG_SPIRAM_SPEED_200M=y
CONFIG_SPIRAM_XIP_FROM_PSRAM=y
CONFIG_SPIRAM_RODATA=y
CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y
```

To:
```
CONFIG_SPIRAM_SPEED_80M=y
# CONFIG_SPIRAM_XIP_FROM_PSRAM is not set
# CONFIG_SPIRAM_RODATA is not set
# CONFIG_SPIRAM_FETCH_INSTRUCTIONS is not set
```

Then rebuild:
```bash
rm -rf build sdkconfig
idf.py build
```

---

### Issue: Flash fails with "Permission denied" or "Line in use"

**Solution:** Kill any processes using the serial port:

**macOS:**
```bash
killall cu
pkill -f monitor
lsof | grep usbmodem
```

**Windows:**
Close any open Serial Monitor programs, Arduino IDE, PuTTY, etc.

---

### Issue: Display blank but backlight on

**Possible causes:**
1. **DSI cable not fully inserted** - Re-seat the FPC cable firmly
2. **Display initialization failed** - Check serial monitor for error messages
3. **Wrong PSRAM settings** - See watchdog reset solution above

---

### Issue: Build error "Python module 'X' not found"

**Solution:** Reinstall ESP-IDF Python requirements:

```bash
cd ~/esp/esp-idf
./install.sh esp32p4
```

---

### Issue: Port not found / Permission denied (macOS)

**Solution:** Add your user to dialout group (if needed):

```bash
sudo dseditgroup -o edit -a $USER -t user dialout
```

Then log out and back in.

---

## Quick Reference Card

```bash
# Install ESP-IDF (one-time setup)
brew install python@3.12                  # macOS - install Python
mkdir -p ~/esp/.pybin
ln -sf /usr/local/bin/python3.12 ~/esp/.pybin/python3
export PATH="$HOME/esp/.pybin:$PATH"
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
git clone -b v5.5.1 --depth 1 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
cd ~/esp/esp-idf
./install.sh esp32p4

# Activate ESP-IDF environment (every new terminal)
source ~/esp/esp-idf/export.sh           # macOS/Linux
export.bat                                # Windows

# Find serial port
ls /dev/cu.*                              # macOS
# Check Device Manager                   # Windows

# Build, flash, monitor
idf.py set-target esp32p4                 # First time only
idf.py build                              # Compile
idf.py -p PORT flash                      # Flash to board
idf.py -p PORT monitor                    # View output
idf.py -p PORT flash monitor              # Flash + monitor

# Clean builds
idf.py fullclean                          # Nuclear option
rm -rf build sdkconfig                    # Manual clean
idf.py build                              # Rebuild

# Configuration
idf.py menuconfig                         # GUI config
nano sdkconfig.defaults                   # Edit defaults

# Exit monitor
Ctrl + ]                                  # Exit serial monitor
```

---

## Daily Workflow

1. **Open terminal**
2. **Activate ESP-IDF**:
   ```bash
   source ~/esp/esp-idf/export.sh
   ```
3. **Navigate to project**:
   ```bash
   cd /path/to/your/project
   ```
4. **Make code changes** (edit files with your preferred editor)
5. **Build**:
   ```bash
   idf.py build
   ```
6. **Flash and monitor**:
   ```bash
   idf.py -p PORT flash monitor
   ```
7. **Test on hardware**
8. **Repeat from step 4**

---

## Tips

- **Use `idf.py -p PORT flash monitor`** for fastest iteration (combines flash + monitor)
- **Keep a dedicated terminal** with ESP-IDF environment sourced
- **Use `idf.py fullclean` sparingly** - normal `idf.py build` is incremental and fast
- **Check serial output early** - many issues show clear error messages
- **Re-seat cables** if display issues occur - loose FPC cables cause mysterious failures

---

## Success Checklist

✓ `idf.py --version` shows v5.5.1
✓ `idf.py build` completes without errors
✓ Flash succeeds: "Hash of data verified"
✓ Serial shows: "OSOYOO 10.1" panel ready (800x1280)"
✓ Serial shows: "UI is up. Tap the button to toggle 'Hello'."
✓ Display shows "Hello World!" and button
✓ Touch works (button responds)

---

## Further Reading

- ESP-IDF Programming Guide: https://docs.espressif.com/projects/esp-idf/en/v5.5.1/
- ESP-IDF Build System: https://docs.espressif.com/projects/esp-idf/en/v5.5.1/api-guides/build-system.html
- ESP32-P4 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32-p4_technical_reference_manual_en.pdf
