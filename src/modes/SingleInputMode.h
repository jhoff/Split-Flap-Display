#pragma once

#include "DisplayMode.h"

class SingleInputMode : public DisplayMode {
public:
    using DisplayMode::DisplayMode;

    void update() override {
        String userInput = webServer.getInputString();
        if (userInput != webServer.getWrittenString()) {
            display.writeString(userInput, MAX_RPM, webServer.getCentering());
            webServer.setWrittenString(userInput);
        }
    }
};
