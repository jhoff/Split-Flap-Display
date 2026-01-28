# MQTT Accuracy Issue - Diagnosis & Solution

## Problem Statement
Display is less accurate when receiving MQTT messages compared to multi-word cycling mode.

## Root Causes Identified

### 1. **No Centering with MQTT**
**Location**: `SplitFlapMqtt.cpp:31`
```cpp
display->writeString(message, maxVel, false);  // centering hardcoded to FALSE
```
**Issue**: Non-centered text requires exact positioning. Centered text is more forgiving of slight errors.

### 2. **No Movement Completion Check**
**Issue**: MQTT can send new message while display is still moving from previous command.
**Result**: Movement interrupted mid-operation, motors don't complete settling.

### 3. **No Rate Limiting**
**Issue**: Rapid MQTT messages can overwhelm the display.
**Comparison**: Multi-word mode has built-in delay between words.

### 4. **Interrupt Timing Jitter**
**Issue**: MQTT callback can fire during critical motor control timing loops.
**Result**: Microsecond-level timing disruptions affect step accuracy.

---

## Proposed Solutions

### Solution 1: Add Movement Busy Flag (Recommended)
**Impact**: High - Prevents command overlap
**Complexity**: Low

```cpp
// In SplitFlapDisplay.h - add:
bool isBusy() { return isMoving; }

// In SplitFlapDisplay.cpp - moveTo() function:
isMoving = true;
// ... movement logic ...
isMoving = false;

// In SplitFlapMqtt.cpp - callback:
if (display && !display->isBusy()) {
    display->writeString(message, maxVel, centering);
} else {
    Serial.println("[MQTT] Display busy, queuing message");
    pendingMessage = message;
    hasPendingMessage = true;
}

// In SplitFlapMqtt.cpp - loop():
if (hasPendingMessage && display && !display->isBusy()) {
    display->writeString(pendingMessage, maxVel, centering);
    hasPendingMessage = false;
}
```

### Solution 2: Make Centering Configurable
**Impact**: Medium - Allows user preference
**Complexity**: Low

```cpp
// Add to JsonSettings:
{"mqtt_centering", JsonSetting(true)},

// In SplitFlapMqtt.cpp:
bool centering = settings.getBool("mqtt_centering");
display->writeString(message, maxVel, centering);
```

### Solution 3: Add Settling Delay
**Impact**: Medium - Ensures motors complete movement
**Complexity**: Very Low

```cpp
// In SplitFlapMqtt.cpp callback after writeString():
delay(200);  // Allow motors to settle before accepting next command
```

### Solution 4: Debounce MQTT Messages
**Impact**: Medium - Prevents rapid-fire commands
**Complexity**: Low

```cpp
// In SplitFlapMqtt.h:
unsigned long lastMqttMessageTime = 0;
const unsigned long MQTT_DEBOUNCE_MS = 500;

// In callback:
unsigned long now = millis();
if (now - lastMqttMessageTime < MQTT_DEBOUNCE_MS) {
    Serial.println("[MQTT] Message ignored (debouncing)");
    return;
}
lastMqttMessageTime = now;
```

---

## Recommended Implementation Order

1. **Solution 3** (Settling Delay) - Immediate, simple fix
2. **Solution 1** (Busy Flag) - Proper long-term solution
3. **Solution 2** (Configurable Centering) - User preference
4. **Solution 4** (Debouncing) - Additional safety net

---

## Testing Plan

1. Send rapid MQTT messages and verify no overlapping movements
2. Test with both centered and non-centered text
3. Compare accuracy between MQTT and multi-word mode
4. Monitor serial output for "Display busy" messages
5. Verify queued messages are processed correctly
