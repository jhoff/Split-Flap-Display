// Split Flap Display
// Morgan Manly 02/16/2025
// Jordan Hoff 03/25/2025
// Thom Koopman 03/30/2025

// Enjoy :)
#include "JsonSettings.h"
#include "SplitFlapDisplay.h"
#include "SplitFlapMqtt.h"
#include "SplitFlapWebServer.h"

#include <Arduino.h>
#include <WiFiClient.h>

// clang-format off
JsonSettings settings = JsonSettings("config", {
    // General Settings
    {"name", JsonSetting("My Display")},
    {"mdns", JsonSetting("splitflap")},
    {"otaPass", JsonSetting("")},
    {"timezone", JsonSetting("UTC0")},
    {"dateFormat", JsonSetting("{dd}-{mm}-{yy}")},
    {"timeFormat", JsonSetting("{HH}:{mm}")},
    // Wifi Settings
    {"ssid", JsonSetting("")},
    {"password", JsonSetting("")},
    // MQTT Settings
    {"mqtt_server", JsonSetting("")},
    {"mqtt_port", JsonSetting(1883)},
    {"mqtt_user", JsonSetting("")},
    {"mqtt_pass", JsonSetting("")},
    // Hardware Settings
    {"moduleCount", JsonSetting(8)},
    {"moduleAddresses", JsonSetting({0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27})},
    {"magnetPosition", JsonSetting(730)},
    {"moduleOffsets", JsonSetting({0, 0, 0, 0, 0, 0, 0, 0})},
    {"displayOffset", JsonSetting(0)},
    {"sdaPin", JsonSetting(8)},
    {"sclPin", JsonSetting(9)},
    {"stepsPerRot", JsonSetting(2048)},
    {"maxVel", JsonSetting(15.0f)},
    {"charset", JsonSetting(48)},
    {"custom_charset", JsonSetting(" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789':?!.-/@$#%")},
    // Operational States
    {"mode", JsonSetting(0)}
});
// clang-format on

WiFiClient wifiClient;
SplitFlapDisplay display(settings);
SplitFlapWebServer webServer(settings);
SplitFlapMqtt splitflapMqtt(settings, wifiClient);

void setup() {
    // put your setup code here, to run once:
    Serial.begin(SERIAL_SPEED);

#ifdef STARTUP_DELAY
    delay(STARTUP_DELAY);
#endif

    Serial.println("Init Web Server");
    webServer.init();

    if (! webServer.connectToWifi()) {
        webServer.startAccessPoint();
        webServer.enableOta();
        webServer.startMDNS();
        webServer.startWebServer();

        display.init();
        display.homeToString("");

        if (display.getNumModules() == 8) {
            display.writeString("Wifi Err");
        } else {
            display.writeChar('X');
        }
    } else {
        webServer.enableOta();
        webServer.startMDNS();
        webServer.startWebServer();

        display.init();
        splitflapMqtt.setup();
        splitflapMqtt.setDisplay(&display);
        display.setMqtt(&splitflapMqtt);

        display.homeToString("OK");
        delay(250);
        display.writeString("");
    }
}

void loop() {
    splitflapMqtt.loop();

    // check what mode the display is in, this value is updated by the web server
    switch (webServer.getMode()) {
        case 0: singleInputMode(); break;
        case 1: multiInputMode(); break;
        case 2: dateMode(); break;
        case 3: timeMode(); break;
        case 4: break;
        case 5: randomTest(); break;
        default: break;
    }

    webServer.handleOta();
    checkConnection();

    reconnectIfNeeded();

    webServer.checkRebootRequired();
    yield();
}

void singleInputMode() {
    String userInput = webServer.getInputString();
    if (userInput != webServer.getWrittenString()) {
        display.writeString(userInput, MAX_RPM, webServer.getCentering());
        webServer.setWrittenString(userInput);
    }
}

void multiInputMode() {
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

void dateMode() {
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

void timeMode() {
    if (millis() - webServer.getLastCheckDateTime() > webServer.getDateCheckInterval()) {
        webServer.setLastCheckDateTime(millis());

        // Get user-friendly format from settings (fallback to "HH:mm")
        String userFormat = settings.getString("timeFormat").length() > 0 ? settings.getString("timeFormat") : "HH:mm";

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

void randomTest() {
    display.testRandom();
    delay(2500);
}

void checkConnection() {
    if (millis() - webServer.getLastCheckWifiTime() >
        webServer.getWifiCheckInterval()) { // check wifi to see if disconnected
        webServer.checkWiFi();
        webServer.setLastCheckWifiTime(millis());
    }
}

void reconnectIfNeeded() {
    if (webServer.getAttemptReconnect()) { // check if the device should attempt reconnection to wifi
        webServer.setAttemptReconnect(false);
        display.writeString("");
        if (! webServer.connectToWifi()) {
            webServer.startAccessPoint();
            webServer.enableOta();
            webServer.endMDNS();
            webServer.startMDNS();
            display.writeChar('X');
        } else {
            webServer.enableOta();
            webServer.endMDNS();
            webServer.startMDNS();
            display.writeString("OK");
            webServer.setWrittenString("OK");
            delay(500);
            display.writeString("");
            webServer.setWrittenString("");
        }

        splitflapMqtt.setup();
    }
}

String extractFromCSV(String str, int index) {
    int startIndex = 0;
    int endIndex = str.length();

    int commaCount = 0;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ',') {
            commaCount++;
            if (commaCount == index) {
                startIndex = i + 1; // skip past the comma
            } else if (commaCount == index + 1) {
                endIndex = i;
            }
        }
    }

    return str.substring(startIndex, endIndex);
}

String renderDate(const String &format) {
    char buf[64];
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    strftime(buf, sizeof(buf), format.c_str(), timeinfo);

    return trimToModuleCount(String(buf), display.getNumModules());
}

String renderTime(const String &format) {
    char buf[64];
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    strftime(buf, sizeof(buf), format.c_str(), timeinfo);

    return trimToModuleCount(String(buf), display.getNumModules());
}

String trimToModuleCount(const String &str, int maxLen) {
    return str.length() > maxLen ? str.substring(0, maxLen) : str;
}

String convertToStrftime(String userFormat) {
    struct FormatToken
    {
        const char *token;
        const char *strftime;
    };

    FormatToken tokens[] = {
        // Date formats
        {"{yyyy}", "%Y"}, // 4-digit year (e.g. 2025)
        {"{dddd}", "%A"}, // Full weekday name (e.g. Monday)
        {"{mmmm}", "%B"}, // Full month name (e.g. January)
        {"{ddd}", "%a"},  // Abbreviated weekday name (e.g. Mon)
        {"{mmm}", "%b"},  // Abbreviated month name (e.g. Apr)
        {"{dd}", "%d"},   // 2-digit day of month, zero-padded (01–31)
        {"{mm}", "%m"},   // 2-digit month number, zero-padded (01–12)
        {"{yy}", "%y"},   // 2-digit year (e.g. 25)
        {"{ww}", "%V"},   // ISO 8601 week number (01–53)
        {"{D}", "%j"},    // Day of the year (001–366)

        // Time formats
        {"{HH}", "%H"},   // Hours (24-hour clock, 00–23)
        {"{hh}", "%I"},   // Hours (12-hour clock, 01–12)
        {"{MM}", "%M"},   // Minutes (00–59)
        {"{AMPM}", "%p"}, // AM or PM
    };

    for (auto &t : tokens) {
        userFormat.replace(t.token, t.strftime);
    }

    return userFormat;
}
