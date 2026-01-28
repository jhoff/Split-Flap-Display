# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Split Flap Display firmware for ESP32-based modular split-flap displays with web interface control and MQTT support.

## Development Commands

### Build and Upload
```bash
# Full build (format, assets, firmware, filesystem)
npm run build

# Individual components
npm run pio:firmware    # Upload firmware only
npm run pio:filesystem  # Upload filesystem only
npm run pio             # Upload both firmware and filesystem
```

### Code Formatting
```bash
npm run format          # Format all code
npm run format:cpp      # Format C++ files (requires clang-format)
npm run format:web      # Format web assets with Prettier
```

### PlatformIO Commands
```bash
# Target specific board environment
pio run -t upload -e esp32_c3       # Default ESP32-C3
pio run -t upload -e esp32_s3       # ESP32-S3
pio run -t uploadfs -e <environment> # Upload filesystem only

# Monitor serial output
npm run pio:monitor
```

### OTA (Over-The-Air) Updates

**Setup Process:**
1. Set an OTA password in the web interface settings page (`Settings > General > OTA Password`)
2. The password is stored in the `otaPass` setting - if empty, OTA is disabled
3. When a password is set, the system requires a restart for OTA to become active

**How OTA Works:**
- `enableOta()` is called during startup if `otaPass` is configured
- Uses ArduinoOTA library with the mDNS hostname (default: `splitflap.local`)
- `handleOta()` is called in the main loop to process incoming OTA requests
- Supports both firmware (`U_FLASH`) and filesystem (`U_LITTLEFS`) updates
- During filesystem updates, LittleFS is unmounted/remounted automatically

**Uploading via OTA:**
```bash
# Use environment with _ota suffix
pio run -t upload -e esp32_c3_ota      # For ESP32-C3
pio run -t upload -e esp32_s3_ota      # For ESP32-S3
pio run -t uploadfs -e esp32_c3_ota    # Upload filesystem only

# Configure authentication in platformio.ini
upload_flags = --auth=yourpassword     # Must match web interface password
```

**PlatformIO OTA Configuration:**
- `upload_protocol=espota` - Uses ESP OTA protocol
- `upload_port=splitflap.local` - Default mDNS hostname (customizable in settings)
- Authentication required if password is set in device settings

**Important Notes:**
- OTA password changes require a device restart to take effect
- Progress is displayed via serial output during updates
- Network must be stable during upload to prevent corruption
- Device automatically reboots after successful OTA update

## Operation Modes

The display supports multiple operation modes controlled via web interface:
- **Mode 0**: Single input mode - displays a single text string
- **Mode 1**: Multi input mode - cycles through multiple text strings with delay
- **Mode 2**: Date mode - displays current date
- **Mode 3**: Time mode - displays current time
- **Mode 4**: Idle mode - no operation
- **Mode 5**: Random test mode - cycles through random characters
- **Mode 6**: Manual mode with special commands
  - Accepts normal text input like mode 0
  - Special command: `#home` - triggers homing sequence to reset all modules
  - Command is cleared after execution

## Architecture

### Core Components

**Main Application** (`src/SplitFlapDisplay.ino`)
- Entry point initializing display, web server, and MQTT
- Configuration via JsonSettings with persistent storage

**SplitFlapDisplay** (`src/SplitFlapDisplay.h/.cpp`)
- Central controller managing up to 8 split-flap modules
- Handles string writing, positioning, and homing operations
- Interfaces with individual modules via I2C

**SplitFlapModule** (`src/SplitFlapModule.h/.cpp`)  
- Controls individual split-flap units via PCF8575 I2C expander
- Manages stepper motor control and Hall effect sensor feedback
- Supports 37 or 48 character sets

**SplitFlapWebServer** (`src/SplitFlapWebServer.h/.cpp`)
- ESPAsyncWebServer-based web interface on port 80
- REST API endpoints for display control and configuration
- Serves minified web assets from LittleFS

**SplitFlapMqtt** (`src/SplitFlapMqtt.h/.cpp`)
- MQTT client for remote display control
- Publishes status updates and subscribes to command topics

**JsonSettings** (`src/JsonSettings.h/.cpp`)
- Persistent configuration storage in LittleFS
- Manages WiFi, MQTT, hardware, and operational settings

### Web Assets

Located in `src/web/`, built with Vite:
- `index.html/js` - Main control interface with Alpine.js
- `settings.html` - Configuration interface
- Tailwind CSS for styling
- Assets minified and copied to `.pio/build/littlefs/` during build

### Build Process

1. Vite builds and minifies web assets to `build/web/`
2. `build/scripts/gzip_littlefs.py` compresses assets to `.pio/build/littlefs/`
3. PlatformIO compiles firmware and uploads with LittleFS filesystem

## Hardware Configuration

- **I2C Communication**: Configurable SDA/SCL pins (default 8/9)
- **Module Addresses**: PCF8575 at 0x20-0x27 (configurable)
- **Stepper Control**: 2048 steps per rotation (default)
- **Position Feedback**: Hall effect sensor for homing
- **Character Sets**: 37 standard or 48 extended characters

### Module Offset Feature

The module offset system compensates for mechanical variations between split-flap modules:

**Two Types of Offsets:**
1. **moduleOffsets** (array): Individual offset for each module (default: `[0,0,0,0,0,0,0,0]`)
   - Applied per-module to adjust for mechanical differences
   - Configured in settings as comma-separated values
   - Example: `[10, -5, 0, 15, 0, 0, -10, 20]` adjusts individual modules

2. **displayOffset** (single value): Global offset applied to all modules (default: `0`)
   - Shifts the entire display's reference position
   - Useful for adjusting all modules together

**How Offsets Work:**
- During initialization (`SplitFlapDisplay.cpp:36`), each module receives: `moduleOffsets[i] + displayOffset`
- This combined offset adjusts the `magnetPosition` in `SplitFlapModule` constructor
- The adjusted position becomes: `magnetPosition = magnetPos + stepOffset`
- Offsets are in motor steps (out of 2048 per rotation)
- Positive values rotate the reference position clockwise
- Negative values rotate counter-clockwise

**Practical Use:**
- If a module's flap doesn't align properly when homed, adjust its `moduleOffset`
- If the entire display is off by the same amount, adjust `displayOffset`
- Fine-tune via web interface settings page or MQTT configuration

**Dynamic Offset Updates:** 
- Module offsets can now be updated dynamically without requiring a system restart
- When `moduleOffsets` or `displayOffset` settings are changed via the web interface, the system automatically calls `display.updateOffsets()`
- The `updateOffsets()` method reloads offset values from settings and applies them to each module immediately
- Each module stores both `baseMagnetPosition` (original) and `magnetPosition` (with offset applied) to enable dynamic updates
- The web server maintains a pointer to the display object to trigger updates when offset settings change