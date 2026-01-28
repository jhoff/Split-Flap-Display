#pragma once

#include "DisplayMode.h"

class ManualMode : public DisplayMode {
public:
    using DisplayMode::DisplayMode;

    void update() override {
        String userInput = webServer.getInputString();
        
        // Check for #home command
        if (userInput == "#home") {
            Serial.println("Homing display...");
            display.home();
            webServer.setInputString("");  // Clear the command after execution
            webServer.setWrittenString("");
        } else if (userInput != webServer.getWrittenString() && userInput != "") {
            // Normal text display
            display.writeString(userInput, MAX_RPM, webServer.getCentering());
            webServer.setWrittenString(userInput);
        }
    }
};
