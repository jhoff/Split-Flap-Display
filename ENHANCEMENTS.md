# Split-Flap Display Enhancements

This document describes the enhancements and new features added to the Split-Flap Display project.

---

## Table of Contents

1. [Module Calibration Interface](#module-calibration-interface)
2. [Manual Homing Command](#manual-homing-command)
3. [I2C Connectivity Testing](#i2c-connectivity-testing)
4. [Dynamic Offset Updates](#dynamic-offset-updates)
5. [Automatic Restart on Module Count Change](#automatic-restart-on-module-count-change)
6. [Watchdog Timer Protection](#watchdog-timer-protection)
7. [Synchronized Landing Transition](#synchronized-landing-transition)
8. [Configuration Overrides](#configuration-overrides)
9. [Time & Date Formatting](#time--date-formatting)

---

## Module Calibration Interface

### Overview
A web-based interface for calibrating individual split-flap modules directly from the settings page. This allows real-time adjustment of module offsets to ensure proper character alignment.

### Location
**Settings Page → Hardware Settings → Module Calibration**

### Features

#### Visual Calibration Controls
Each module gets its own calibration row with:
- **Module identifier** (Module 0, Module 1, etc.)
- **Offset adjustment buttons**: -10, -5, -1, +1, +5, +10
- **Current offset display**: Shows the current offset value in steps
- **Test button**: Homes the module and displays a blank character to verify alignment

#### How to Use

1. **Navigate to Settings**
   - Open `http://splitflap.local/settings.html` in your browser
   - Scroll down to the "Module Calibration" section

2. **Test a Module**
   - Click the blue **Test** button for a module
   - The module will:
     - Home to the magnet sensor (reference position)
     - Move to display a blank character
     - This allows you to visually check alignment

3. **Adjust Offset**
   - If the blank character is misaligned, use the +/- buttons
   - Small adjustments: Use **±1** buttons
   - Medium adjustments: Use **±5** buttons
   - Large adjustments: Use **±10** buttons
   - The offset updates immediately and applies to the module

4. **Verify Adjustment**
   - Click **Test** again to re-home with the new offset
   - Observe if the alignment improved
   - Repeat until perfectly aligned

5. **Save Settings**
   - Once all modules are calibrated, click **Save Settings** at the bottom
   - This persists the offset values to flash storage

### Technical Details

**Offset Values:**
- Measured in motor steps (out of 2048 per rotation)
- Positive values: Rotate reference position clockwise
- Negative values: Rotate reference position counter-clockwise
- Range: Typically -100 to +100 steps for fine-tuning

**API Endpoints:**
- `POST /api/module/{index}/offset` - Update offset for a specific module
  ```json
  {
    "offset": 25
  }
  ```
- `POST /api/module/{index}/test` - Test/home a specific module

**Help Documentation:**
Click the **?** icon next to "Module Calibration" for in-interface help.

---

## Manual Homing Command

### Overview
A special command that allows manual triggering of the homing sequence for all modules simultaneously. This resets all modules to their reference positions using the magnet sensors.

### Location
**Main Display Page → Text Input Field**

### How to Use

1. **Navigate to Main Page**
   - Open `http://splitflap.local` in your browser
   - Locate the text input field where you normally enter display text

2. **Enter Homing Command**
   - Type `#home` (case-sensitive) into the text input field
   - Click the submit button or press Enter

3. **Observe Homing Sequence**
   - All modules will begin rotating to find their magnet sensor positions
   - Each module stops when it detects the magnet (reference position)
   - The command is automatically cleared after execution
   - Input field returns to normal text mode

### When to Use

**After Power Loss or Reset:**
- Ensures modules start from known positions
- Reestablishes accurate character alignment

**After Manual Movement:**
- If modules were physically moved or rotated
- Resets to calibrated reference positions

**During Troubleshooting:**
- Verify all modules can find their magnet sensors
- Check that homing sequence completes without errors
- Diagnose mechanical or sensor issues

**After Offset Calibration:**
- Verify new offsets are applied correctly
- Confirm modules home to expected positions

### Technical Details

**Command Format:**
- Command: `#home` (must be exactly this string, lowercase)
- Processing: Handled in Mode 6 (Manual mode)
- Execution: Calls `display.home()` to initiate homing sequence

**Homing Process:**
1. All modules begin rotating backward (counter-clockwise)
2. Each module monitors its Hall effect sensor
3. When magnet is detected, module stops immediately
4. Module position is set to `magnetPosition` (includes offsets)
5. Command is cleared from input field after completion

**Code Reference:**
Located in `src/SplitFlapDisplay.ino` loop function (mode 6 handling):
```cpp
if (mode == 6 && text != lastText) {
    if (text == "#home") {
        display.home();
        text = "";  // Clear command after execution
    } else {
        display.writeText(text);
    }
    lastText = text;
}
```

**Safety Features:**
- Watchdog timer feeding during homing prevents system resets
- Each module homes independently to avoid mechanical stress
- Offset values are preserved and applied after homing

### Difference from Individual Module Testing

**Manual Homing Command (`#home`):**
- Homes ALL modules simultaneously
- Only performs homing, no character display
- Used for full display reset
- Triggered via text input on main page

**Module Test (Calibration Interface):**
- Homes ONE specific module
- Displays blank character after homing to verify alignment
- Used during calibration and troubleshooting
- Triggered via Test button on settings page

### Example Use Case

**Scenario:** Device was unplugged and modules drifted out of position

1. Power on the device and connect to `http://splitflap.local`
2. Notice characters are misaligned or incorrect
3. Type `#home` in the text input field
4. Press Enter
5. Wait for all modules to complete homing (~5-10 seconds)
6. Display is now reset to reference positions
7. Type normal text to verify proper operation

---

## I2C Connectivity Testing

### Overview
Verify I2C communication with each split-flap module to diagnose hardware connectivity issues.

### API Endpoint
```
GET http://splitflap.local/api/i2c/test
```

### Response Format
```json
{
  "modules": [
    {
      "index": 0,
      "address": 32,
      "connected": true
    },
    {
      "index": 1,
      "address": 33,
      "connected": false
    },
    {
      "index": 2,
      "address": 34,
      "connected": true
    }
  ],
  "message": "I2C connectivity test complete",
  "type": "success"
}
```

### How to Use

#### Method 1: Browser
1. Open your browser
2. Navigate to: `http://splitflap.local/api/i2c/test`
3. View the JSON response showing connectivity status

#### Method 2: Command Line
```bash
curl http://splitflap.local/api/i2c/test
```

#### Method 3: Browser Console
```javascript
fetch('/api/i2c/test')
  .then(r => r.json())
  .then(data => {
    console.table(data.modules);
    data.modules.forEach(m => {
      console.log(`Module ${m.index} (0x${m.address.toString(16)}): ${m.connected ? '✓' : '✗'}`);
    });
  });
```

### What It Tests

The connectivity test verifies:
1. **I2C Write**: Attempts to begin transmission to the module's address
2. **I2C Read**: Attempts to read 2 bytes from the module
3. Both operations must succeed for `connected: true`

### Troubleshooting

If a module shows `connected: false`:

**Check Physical Connections:**
- Verify SDA and SCL wiring to the module
- Ensure module has proper power (check voltage)
- Look for loose or damaged connectors

**Check I2C Configuration:**
- Verify the module address matches settings
- Default addresses: 32 (0x20), 33 (0x21), 34 (0x22), etc.
- No two modules should share the same address

**Check I2C Bus:**
- Look for shorts or interference on SDA/SCL lines
- Ensure pull-up resistors are present (usually on PCF8575 boards)
- Try reducing I2C clock speed if experiencing issues

**Hardware Issues:**
- Test the PCF8575 chip on the module
- Check for damaged components
- Verify the module works when tested individually

---

## Dynamic Offset Updates

### Overview
Module offsets can now be updated in real-time without requiring a system restart. Changes apply immediately when using the Module Calibration interface.

### How It Works

**Previous Behavior:**
- Changing offsets required saving settings and restarting the ESP32
- Modules retained old offset values until restart

**New Behavior:**
- Offset changes apply immediately via `updateOffsets()` function
- Each module stores both `baseMagnetPosition` (original) and `magnetPosition` (with offset)
- When offset changes, `magnetPosition` is recalculated: `magnetPosition = baseMagnetPosition + newOffset`

### Implementation Details

**Module Offset Storage:**
- `moduleOffsets[i]` - Individual offset for each module (array)
- `displayOffset` - Global offset applied to all modules (single value)
- Combined offset: `moduleOffsets[i] + displayOffset`

**When Offsets Update:**
1. User adjusts offset in web interface
2. `POST /api/module/{index}/offset` is called
3. New offset is saved to settings
4. `display.updateOffsets()` is called automatically
5. Offset applies to the module immediately
6. Next test/movement uses the new offset

### Manual Offset Update

If you change offsets in settings without using the calibration interface:
- Changes are saved but not applied until restart
- OR trigger manual update by testing the module
- OR restart the device for full reinitialization

---

## Automatic Restart on Module Count Change

### Overview
Changing the number of modules now automatically triggers a system restart, ensuring the hardware configuration matches the software configuration.

### Why It's Needed

**Module Count Impact:**
- Affects memory allocation for module arrays
- Determines I2C initialization scope
- Impacts loop iterations throughout the codebase
- Cannot be changed dynamically without full reinitialization

### How It Works

**Previous Behavior:**
- User changes `moduleCount` in settings
- Setting saves to flash
- Old value remains in RAM until manual restart
- Unused modules still receive commands

**New Behavior:**
1. User changes "Number of Modules" in settings
2. Clicks "Save Settings"
3. System detects `moduleCount` change
4. Displays message: "Settings updated successfully, Module count has changed. Rebooting..."
5. ESP32 automatically restarts (~10 seconds)
6. On restart, new `moduleCount` loads from flash
7. Only the specified number of modules are initialized and controlled

### Usage Example

**Scenario:** You have 3 modules but only want to use 2

1. Go to Settings → Hardware Settings
2. Change "Number of Modules" from 3 to 2
3. Click "Save Settings"
4. Wait for automatic restart message
5. Device reboots in ~10 seconds
6. After restart, only Module 0 and Module 1 are controlled
7. Module 2 (address 34) no longer receives commands

### Other Settings That Trigger Restart

The following settings also trigger automatic restart:
- **OTA Password**: Security change requires restart to apply
- **mDNS Hostname**: Network identification change requires restart

### Implementation Details

**Backend (C++):**
```cpp
if (json["moduleCount"].is<int>() &&
    json["moduleCount"].as<int>() != settings.getInt("moduleCount")) {
    rebootRequired = true;
    response["message"] = "Settings updated successfully, Module count has changed. Rebooting...";
}
```

**Settings Handler:**
- Compares incoming `moduleCount` with stored value
- Sets `rebootRequired` flag if different
- Triggers `ESP.restart()` after response is sent

---

## Watchdog Timer Protection

### Overview
Long-running motor operations (homing, testing) are now protected from watchdog timer resets by feeding the timer periodically during execution.

### The Problem

**ESP32 Watchdog Timer:**
- Monitors tasks to ensure they don't freeze
- Default timeout: 5 seconds for most tasks
- If a task doesn't respond within timeout, system resets

**Previous Behavior:**
- Motor control loops ran continuously during homing
- Could take 5+ seconds for slow movements or multiple rotations
- Async TCP task didn't get CPU time
- Watchdog triggered, causing unexpected reboots

**Symptom:**
```
E (62487) task_wdt: Task watchdog got triggered. The following tasks did not reset the watchdog in time:
E (62487) task_wdt:  - async_tcp (CPU 0)
```

### The Solution

**Watchdog Feeding in Motor Loop:**
```cpp
bool isFinished = checkAllFalse(needsStepping, numModules);
unsigned long lastWatchdogFeed = millis();

while (!isFinished) {
    currentTime = micros();

    // Feed the watchdog timer every 100ms to prevent timeout
    if (millis() - lastWatchdogFeed > 100) {
        yield();                // Allow other tasks to run
        esp_task_wdt_reset();   // Reset the task watchdog timer
        lastWatchdogFeed = millis();
    }

    // ... motor control code ...
}
```

### How It Works

1. **Track Time**: `lastWatchdogFeed` stores last feed time
2. **Check Interval**: Every loop iteration checks if 100ms has passed
3. **Yield**: Calls `yield()` to give CPU time to other tasks
4. **Feed Watchdog**: Calls `esp_task_wdt_reset()` to reset the timer
5. **Continue**: Motor control continues without interruption

### Impact

**Before:**
- System crashes during long homing operations
- Web interface becomes unresponsive
- MQTT disconnects during movements

**After:**
- Motor operations complete successfully
- Web interface remains responsive
- No watchdog-related crashes
- Network tasks continue running

### Additional Delays

**Test Module Function:**
Also includes yield protection in delays:
```cpp
// Use smaller delays with yields to avoid watchdog timeout
for (int i = 0; i < 5; i++) {
    delay(100);
    yield();  // Allow other tasks to run
}
```

This ensures even the 500ms delay between homing and moving to blank doesn't cause issues.

---

## Synchronized Landing Transition

### Overview
A visually pleasing transition effect where modules with shorter travel distances wait to start moving, ensuring that all modules stop arriving at their target character at the exact same moment.

### How to Enable
1. Go to **Settings**
2. Change **Transition Style** to **Synchronized Landing**
3. Save Settings

### Technical Details
- Calculates travel distance for every module
- Determines the "longest path" (maximum travel time)
- Delays shorter paths by `(max_steps - my_steps) * time_per_step`
- Uses non-blocking delays (`delay(1)` with `yield()`) to prevent watchdog timeouts during long waits

---

## Configuration Overrides

### Overview
Allows advanced users to customize hardware settings (Module Addresses and Offsets) without modifying the source code directly, preserving these settings across project updates.

### How to Use
1. Create a file named `platformio_override.ini` in the project root
2. Add your custom definitions using build flags:

```ini
[env:esp32_c3]
build_flags =
    ${env.build_flags}
    ; Override Module Addresses
    -D DEFAULT_MODULE_ADDRESSES="{0x20, 0x21, 0x22}"
    ; Override Module Offsets
    -D DEFAULT_MODULE_OFFSETS="{0, 0, 0}"
```

3. Rebuild and upload the firmware. The system will use your lists instead of the hardcoded defaults.

---

## Time & Date Formatting

### Overview
Significant improvements have been made to the Time and Date modes to ensure they actually respect the user's settings and handle synchronization states gracefully.

### Key Fixes
1.  **Configurable Formats**: The display now correctly uses the `timeFormat` and `dateFormat` settings defined in the web interface. 
    - Fallback: Defaults to `{HH}:{mm}` and `{dd}-{MM}-{yy}` if settings are missing.
2.  **Format Conversion**: A helper `convertToStrftime` automatically translates user-friendly tokens (like `{HH}`) into standard C++ `strftime` format specifiers.
3.  **NTP Synchronization Check**: 
    - Prevents displaying invalid times (e.g., 1970) on boot.
    - Displays `--:--` until accurate time is received from NTP servers.
4.  **Immediate Updates**: `TimeMode::enter()` now forces an immediate display update, removing lag when switching modes.

---

## Summary of API Endpoints

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/api/module/{index}/test` | POST | Home and test a specific module |
| `/api/module/{index}/offset` | POST | Update offset for a specific module |
| `/api/i2c/test` | GET | Test I2C connectivity for all modules |

---

## Configuration Files

**Settings Location:**
- Flash storage via `Preferences` library
- Key: `settings`
- Format: JSON

**Relevant Settings:**
```json
{
  "moduleCount": 3,
  "moduleOffsets": "0,-30,-45,0,0,0,0,0",
  "displayOffset": 0,
  "moduleAddresses": "32,33,34,35,36,37,38,39"
}
```

---

## Browser Compatibility

All enhancements work with modern browsers:
- Chrome/Edge (recommended)
- Firefox
- Safari
- Any browser with JavaScript enabled

---

## Future Enhancements (Potential)

- Visual character alignment indicators
- Batch offset adjustment (apply same offset to all modules)
- Offset presets/profiles
- Auto-calibration using hall sensor feedback
- I2C bus scanning to detect all connected devices
- Connectivity monitoring dashboard

---

## Credits

Enhancements developed using Claude Code for the Split-Flap Display project.

**Original Project:** https://github.com/kwadra/Split-Flap-Display

---

## License

These enhancements follow the same license as the original Split-Flap Display project.
