#pragma once

#include "DisplayMode.h"
#include "../Utils.h"

class DateMode : public DisplayMode {
public:
    using DisplayMode::DisplayMode;

    void update() override {
        if (millis() - webServer.getLastCheckDateTime() > webServer.getDateCheckInterval()) {
            webServer.setLastCheckDateTime(millis());

            String format = settings.getString("dateFormat");
            String strftimeFormat = convertToStrftime(format);
            String result = renderDate(strftimeFormat);

            if (result.length() <= display.getNumModules() && result != webServer.getWrittenString()) {
                display.writeString(result, MAX_RPM);
                webServer.setWrittenString(result);
            }
        }
    }

private:
    String renderDate(const String &format) {
        char buf[64];
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);

        strftime(buf, sizeof(buf), format.c_str(), timeinfo);

        return trimToModuleCount(String(buf), display.getNumModules());
    }
};
