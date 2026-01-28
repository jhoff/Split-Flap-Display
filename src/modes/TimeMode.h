#pragma once

#include "DisplayMode.h"
#include "../Utils.h"

class TimeMode : public DisplayMode {
public:
    using DisplayMode::DisplayMode;

    void enter() override {
        // Force an update immediately upon entry by resetting the tracking state
        webServer.setWrittenString(""); 
        webServer.setLastCheckDateTime(0); // Ensure condition fires immediately
    }

    void update() override {
        if (millis() - webServer.getLastCheckDateTime() > webServer.getDateCheckInterval()) {
            webServer.setLastCheckDateTime(millis());

            // Get user-friendly format from settings (fallback to "{HH}:{mm}")
            String userFormat = settings.getString("timeFormat").length() > 0 ? settings.getString("timeFormat") : "{HH}:{mm}";

            // Convert to strftime-compatible format
            String strftimeFormat = convertToStrftime(userFormat);
            String result = renderTime(strftimeFormat);

            // Write to display if it changed
            if (result != webServer.getWrittenString()) {
                display.writeString(result, MAX_RPM);
                webServer.setWrittenString(result);
            }
        }
    }

private:
    String renderTime(const String &format) {
        char buf[64];
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);

        if (!timeinfo) {
            return "Err Time"; // Return error if time failed
        }
        
        // Check if time is valid (year > 2000, i.e., > 100 in tm_year)
        // tm_year is years since 1900
        if (timeinfo->tm_year < 100) {
            return "--:--"; // NTP not synced yet
        }

        strftime(buf, sizeof(buf), format.c_str(), timeinfo);

        return trimToModuleCount(String(buf), display.getNumModules());
    }
};
