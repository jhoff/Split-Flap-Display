#pragma once

#include "DisplayMode.h"

class RandomTestMode : public DisplayMode {
public:
    using DisplayMode::DisplayMode;

    void update() override {
        if (millis() - lastUpdate > 2500) {
            display.testRandom();
            lastUpdate = millis();
        }
    }

private:
    unsigned long lastUpdate = 0;
};
