#pragma once

#include "DisplayMode.h"
#include "../Utils.h"

class MultiInputMode : public DisplayMode {
public:
    using DisplayMode::DisplayMode;

    void update() override {
        if (millis() - webServer.getLastSwitchMultiTime() > webServer.getMultiWordDelay()) {
            // get user input, extract correct word from index using webserver counter, and display
            String userInput = webServer.getMultiInputString();
            String currWord = extractFromCSV(userInput, webServer.getMultiWordCurrentIndex());
            if (currWord != webServer.getWrittenString()) {
                display.writeString(currWord, MAX_RPM, webServer.getCentering());
                webServer.setWrittenString(currWord);
            }
            webServer.setLastSwitchMultiTime(millis());
            webServer.setMultiWordCurrentIndex((webServer.getMultiWordCurrentIndex() + 1) % (webServer.getNumMultiWords()));
        }
    }
};
