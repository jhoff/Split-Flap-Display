# Split-Flap Display - Timing Issues Analysis

## Critical Timing Issues Found

### 🔴 ISSUE #1: Stale `currentTime` in Motor Control Loop
**Location**: `SplitFlapDisplay.cpp:360-372`
**Severity**: HIGH - Affects step timing accuracy

```cpp
while (!isFinished) {
    currentTime = micros();  // Read once at start of loop

    // ... watchdog code ...

    for (int i = 0; i < numModules; i++) {
        if (((currentTime - lastStepTimes[i]) > timePerStep) && needsStepping[i]) {
            modules[i].step();
            lastStepTimes[i] = micros();  // ❌ Using fresh micros() here!
```

**Problem**:
- `currentTime` is read once at loop start
- `lastStepTimes[i]` is updated with **fresh** `micros()` call
- This creates inconsistency - comparing stale time to fresh time
- Can cause irregular step timing, especially with multiple modules

**Impact**: Steps may fire slightly early/late, accumulating timing errors

**Solution**:
```cpp
for (int i = 0; i < numModules; i++) {
    if (((currentTime - lastStepTimes[i]) > timePerStep) && needsStepping[i]) {
        modules[i].step();
        lastStepTimes[i] = currentTime;  // ✅ Use consistent time value
```

---

### 🟡 ISSUE #2: I2C Operations Inside Timing-Critical Loop
**Location**: `SplitFlapDisplay.cpp:380-408`, `SplitFlapModule.cpp:162-180`
**Severity**: MEDIUM - Causes timing jitter

**Problem**:
- Hall effect sensor reading (`readHallEffectSensor()`) performs I2C `Wire.requestFrom()`
- I2C operations take variable time (typically 100-400 microseconds)
- Called every 20ms inside the motor control loop
- Blocks CPU during I2C transaction

**Impact**:
- Variable loop execution time
- Can delay next motor step by 100-400µs
- At 15 RPM (max speed), timePerStep ≈ 4883µs, so 400µs jitter = 8% timing error

**Solution Options**:
1. Accept the jitter (current - mostly acceptable)
2. Use interrupt-driven I2C (complex)
3. Increase step time margin to account for jitter

---

### 🟡 ISSUE #3: Main Loop Can Interrupt Motor Control
**Location**: `SplitFlapDisplay.ino:120-141`
**Severity**: MEDIUM - Causes unpredictable delays

**Problem**:
```cpp
void loop() {
    splitflapMqtt.loop();              // Can trigger MQTT callback
    // ... mode handlers call display.writeString() ...
    webServer.handleOta();             // OTA can take milliseconds
    checkConnection();                 // WiFi operations
    reconnectIfNeeded();               // Can delay 100-1000ms
    webServer.checkRebootRequired();
}
```

**Issues**:
- `splitflapMqtt.loop()` can trigger immediate `writeString()` via callback
- `handleOta()` calls `ArduinoOTA.handle()` which can block
- `reconnectIfNeeded()` has `delay(100)` and `delay(500)` calls
- `checkConnection()` has `delay(100)` calls

**Impact**: Motor control can be interrupted by network operations

**Solution**: Add busy flag to prevent concurrent operations (see MQTT_ACCURACY_FIX.md)

---

### 🟡 ISSUE #4: Watchdog Feeding Disrupts Timing
**Location**: `SplitFlapDisplay.cpp:363-367`
**Severity**: LOW-MEDIUM - Periodic timing hiccups

```cpp
if (millis() - lastWatchdogFeed > WATCHDOG_FEED_INTERVAL_MS) {
    yield();                      // ⚠️ Can take unpredictable time
    esp_task_wdt_reset();
    lastWatchdogFeed = millis();
}
```

**Problem**:
- `yield()` allows other tasks to run
- Can delay return by milliseconds
- Happens every 100ms during movement
- Mixing `millis()` and `micros()` timing

**Impact**: Brief timing hiccup every 100ms during movement

**Solution**:
- Accept it (watchdog is necessary)
- Or track if yield caused significant delay and compensate

---

### 🟠 ISSUE #5: Sensor Check Updates `currentTime` Reference
**Location**: `SplitFlapDisplay.cpp:410-411`
**Severity**: LOW - Inconsistent time tracking

```cpp
isFinished = checkAllFalse(needsStepping, numModules);
lastSensorCheckTime = currentTime; // recall micros because for loop may take a moment
```

**Comment says "recall micros" but actually just assigns `currentTime`**

**Problem**: The comment suggests they wanted to call `micros()` again, but they're using stale `currentTime`

**Impact**: Sensor check interval slightly irregular

**Solution**:
```cpp
lastSensorCheckTime = micros();  // Get fresh timestamp
```

---

### 🟢 ISSUE #6: I2C Error Recovery Uses `delay(10)`
**Location**: `SplitFlapModule.cpp:64-70`
**Severity**: LOW - Rare occurrence

```cpp
if (consecutiveErrors >= 3) {
    Serial.print("Module ");
    Serial.print(address);
    Serial.println(" has persistent I2C errors, attempting recovery...");
    delay(10);  // ⚠️ 10ms delay during motor control
    consecutiveErrors = 0;
}
```

**Problem**: If I2C errors occur during movement, this delay disrupts timing

**Impact**: Very rare, but can cause missed steps during error recovery

**Solution**: Remove delay or make it shorter (1-2ms sufficient)

---

### 🟢 ISSUE #7: No Timeout on I2C `Wire.requestFrom()`
**Location**: `SplitFlapModule.cpp:168-170`
**Severity**: LOW - Potential hang

```cpp
Wire.requestFrom(address, requestBytes);
if (Wire.available() == 2) {  // What if bytes never arrive?
```

**Problem**: If I2C bus hangs, this could wait indefinitely

**Impact**: Extremely rare, but could freeze motor control

**Solution**: Add timeout or check before reading

---

## Timing Analysis Summary

### Motor Control Loop Timing Budget
At **15 RPM** (max speed):
- Steps per rotation: 2048
- Time per step: `60,000,000 µs / (15 * 2048)` = **1,953 µs**

### Current Timing Overhead (per loop iteration):
- `micros()` call: ~5 µs
- Step logic (8 modules): ~40 µs
- Sensor check (every 20ms): ~400 µs (I2C read)
- Watchdog feed (every 100ms): ~100-1000 µs
- **Total typical**: 45-445 µs per iteration

### Jitter Sources:
1. I2C operations: ±200 µs
2. Watchdog yield: ±500 µs (every 100ms)
3. Main loop interruption: ±100-1000 µs (during MQTT/WiFi)
4. Inconsistent time reference: ±10-50 µs

---

## Priority Fixes

### Priority 1: Fix Stale `currentTime` (Issue #1) ✅
**Impact**: Immediate improvement to step timing consistency
**Complexity**: Trivial (one line change)

### Priority 2: Add Movement Busy Flag (Issue #3) ✅
**Impact**: Prevents concurrent operations
**Complexity**: Low (see MQTT_ACCURACY_FIX.md)

### Priority 3: Fix Sensor Check Timestamp (Issue #5) ✅
**Impact**: More consistent sensor polling
**Complexity**: Trivial (one line change)

### Priority 4: Reduce I2C Error Recovery Delay (Issue #6) ✅
**Impact**: Faster recovery without timing disruption
**Complexity**: Trivial (change delay value)

### Priority 5: Document/Accept I2C Jitter (Issue #2) ℹ️
**Impact**: Informational - jitter is mostly acceptable
**Complexity**: N/A (document limits)

---

## Recommended Implementation

1. Fix `lastStepTimes[i]` assignment (Issue #1)
2. Fix `lastSensorCheckTime` assignment (Issue #5)
3. Reduce error recovery delay to 2ms (Issue #6)
4. Add busy flag system (Issue #3)
5. Document timing limitations

These changes will significantly improve timing consistency with minimal code changes.
