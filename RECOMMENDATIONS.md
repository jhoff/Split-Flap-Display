# Project Improvement Recommendations

Based on an analysis of the codebase, here are several recommendations to improve the quality, reliability, and maintainability of the Split Flap Display project.

## Firmware (C++)

### 1. Non-Blocking Architecture
**Issue:** The current implementation uses `delay()` in several places (e.g., `wakeUp`, `reconnectIfNeeded`, `setup`), which pauses the entire processor. This can interfere with network tasks (MQTT/Web Server) and make the interface feel sluggish.
**Recommendation:**
-   Replace `delay()` with non-blocking state machines using `millis()` timers.
-   Implement a `Task` class or use a lightweight scheduler to manage concurrent operations (Motor control, WiFi, MQTT, Web Server).

### 2. State Management Refactoring
**Issue:** The main `loop()` in `SplitFlapDisplay.ino` contains a large switch statement handling various display modes. This makes the main file cluttered and harder to extend.
**Recommendation:**
-   Implement the **State Pattern**. Create a base `DisplayMode` class and subclasses for each mode (`SingleInputMode`, `ClockMode`, `RandomTestMode`, etc.).
-   The main loop would simply call `currentMode->update()`.

### 3. I2C Reliability
**Issue:** `SplitFlapModule.cpp` has error handling, but I2C on ESP32 can be finicky. The current recovery mechanism is basic.
**Recommendation:**
-   Implement a more robust I2C bus recovery sequence (e.g., toggling SCL clocks to clear stuck slaves) if `Wire.endTransmission()` fails repeatedly.
-   Consider using a dedicated I2C library or lower-level driver if the Arduino `Wire` library proves too limiting for error recovery.

### 4. Configuration Management
**Issue:** Configuration keys and defaults are scattered. `JsonSettings` is used, but some defaults are hardcoded in logic.
**Recommendation:**
-   Centralize all default values in a `Defaults.h` file.
-   Ensure all magic numbers (e.g., `710` for magnet position) are named constants.

### 5. String Handling
**Issue:** `SplitFlapWebServer.cpp` manually implements `decodeURIComponent`.
**Recommendation:**
-   Use existing libraries or built-in ESP helper functions for URL decoding to reduce code maintenance and potential bugs.

## Frontend (Web)

### 1. Component Structure
**Issue:** `index.js` is a single large file containing all logic.
**Recommendation:**
-   Split the Alpine.js logic into separate modules (e.g., `settings.js`, `controls.js`, `calibration.js`) and import them. This improves readability and maintainability.

### 2. User Feedback
**Issue:** While there are dialogs, the UI could provide more immediate feedback for connection status or background tasks.
**Recommendation:**
-   Add a persistent status bar showing WiFi and MQTT connection states.
-   Implement optimistic UI updates where appropriate to make the interface feel snappier.

### 3. Build Optimization
**Issue:** The build process involves multiple steps and scripts.
**Recommendation:**
-   Ensure `vite` handles as much of the asset processing as possible.
-   Review `gzip_littlefs.py` to ensure it's efficiently compressing assets for the limited ESP32 storage.

## General

### 1. Documentation
**Issue:** While there are `.md` files for issues, the code itself could benefit from more Javadoc/Doxygen style comments, especially for public methods in classes.
**Recommendation:**
-   Add standard documentation headers to functions in `.h` files explaining parameters and return values.

### 2. Testing
**Issue:** Testing is largely manual.
**Recommendation:**
-   Add unit tests for pure logic functions (like `extractFromCSV`, `convertToStrftime`) using a framework like Unity (built into PlatformIO).
