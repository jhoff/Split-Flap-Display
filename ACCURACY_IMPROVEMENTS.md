# Split-Flap Display - Accuracy Improvements

## Proposed Enhancements for Long-Term Accuracy

### Priority 1: Missed Magnet Detection 🧲
**Problem**: No detection if a module completely misses the magnet during movement.
**Solution**: Track expected vs actual magnet crossings.
**Impact**: Critical safety check, prevents lost position tracking.

### Priority 2: Position Error Statistics 📊
**Problem**: Position errors are corrected but not logged or analyzed.
**Solution**: Track max error, average error, and correction frequency per module.
**Impact**: Diagnostics for maintenance, identify problematic modules.

### Priority 3: Adaptive Speed Control ⚡
**Problem**: Modules with mechanical issues run at same speed as good modules.
**Solution**: Reduce max speed for modules with high error rates.
**Impact**: Immediate accuracy improvement for problematic modules.

### Priority 4: Predictive Homing 🏠
**Problem**: Position drift can accumulate during long display sessions.
**Solution**: Periodically home during idle periods (e.g., every 5 minutes).
**Impact**: Prevents long-term drift, maintains calibration.

### Priority 5: Magnet Position Variance Tracking 📍
**Problem**: Fixed magnetPosition value doesn't adapt to mechanical wear.
**Solution**: Track actual detection positions and auto-adjust magnetPosition.
**Impact**: Self-calibrating system adapts to wear over time.

### Priority 6: Backlash Compensation 🔄
**Problem**: Stepper motors can have directional backlash.
**Solution**: Always approach target from same direction for small movements.
**Impact**: More consistent positioning.

### Priority 7: Temperature Compensation 🌡️
**Problem**: Thermal expansion may affect mechanical accuracy.
**Solution**: Track correlation between temperature and position errors.
**Impact**: Identify environmental factors affecting accuracy.

### Priority 8: Step Verification with Current Sensing ⚡
**Problem**: Missed steps not detected until next magnet pass.
**Solution**: Monitor motor current to detect binding/missed steps.
**Impact**: Real-time error detection (requires hardware modification).

---

## Implementation Notes

- Start with Priority 1-2 (diagnostics, non-invasive)
- Priority 3-5 provide active correction
- Priority 6-8 are advanced features for specific issues

## Code Locations

- `SplitFlapModule.h/cpp` - Module-level tracking
- `SplitFlapDisplay.cpp` - Movement and homing logic
- `SplitFlapWebServer.cpp` - Diagnostics endpoint (future)
