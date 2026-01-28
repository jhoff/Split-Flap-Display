#pragma once

#include "../SplitFlapDisplay.h"
#include "../SplitFlapWebServer.h"
#include "../JsonSettings.h"

class DisplayMode {
public:
    DisplayMode(SplitFlapDisplay& display, SplitFlapWebServer& webServer, JsonSettings& settings)
        : display(display), webServer(webServer), settings(settings) {}
    virtual ~DisplayMode() {}

    virtual void enter() {}
    virtual void update() = 0;
    virtual void exit() {}

protected:
    SplitFlapDisplay& display;
    SplitFlapWebServer& webServer;
    JsonSettings& settings;
};
