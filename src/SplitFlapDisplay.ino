// Split Flap Display
// Morgan Manly 02/16/2025
// Jordan Hoff 03/25/2025
// Thom Koopman 03/30/2025

// Enjoy :)
#include "JsonSettings.h"
#include "SplitFlapDisplay.h"
#include "SplitFlapMqtt.h"
#include "SplitFlapWebServer.h"
#include "Defaults.h"

#include <Arduino.h>
#include <WiFiClient.h>

// Include credentials if file exists (local only, not in git)
#ifdef __has_include
#if __has_include("credentials.h")
#include "credentials.h"
#define HAS_CREDENTIALS
#endif
#endif

// Default credentials (empty - configure via web interface)
#ifndef MQTT_SERVER
#define MQTT_SERVER ""
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#ifndef MQTT_USER
#define MQTT_USER ""
#endif
#ifndef MQTT_PASS
#define MQTT_PASS ""
#endif
#ifndef OTA_PASS
#define OTA_PASS DEFAULT_OTA_PASS
#endif

// clang-format off
JsonSettings settings = JsonSettings("config", {
    // General Settings
    {"name", JsonSetting(DEFAULT_NAME)},
    {"mdns", JsonSetting(DEFAULT_MDNS)},
    {"otaPass", JsonSetting(OTA_PASS)},
    {"timezone", JsonSetting(DEFAULT_TIMEZONE)},
    {"dateFormat", JsonSetting(DEFAULT_DATE_FORMAT)},
    {"timeFormat", JsonSetting(DEFAULT_TIME_FORMAT)},
    // Wifi Settings
    {"ssid", JsonSetting(DEFAULT_SSID)},
    {"password", JsonSetting(DEFAULT_PASSWORD)},
    // MQTT Settings (defaults from credentials.h or empty)
    {"mqtt_server", JsonSetting(MQTT_SERVER)},
    {"mqtt_port", JsonSetting(MQTT_PORT)},
    {"mqtt_user", JsonSetting(MQTT_USER)},
    {"mqtt_pass", JsonSetting(MQTT_PASS)},
    // Hardware Settings
    {"moduleCount", JsonSetting(DEFAULT_MODULE_COUNT)},
    {"moduleAddresses", JsonSetting(std::vector<int>DEFAULT_MODULE_ADDRESSES)},
    {"magnetPosition", JsonSetting(DEFAULT_MAGNET_POSITION)},
    {"moduleOffsets", JsonSetting(std::vector<int>DEFAULT_MODULE_OFFSETS)},
    {"displayOffset", JsonSetting(DEFAULT_DISPLAY_OFFSET)},
    {"sdaPin", JsonSetting(DEFAULT_SDA_PIN)},
    {"sclPin", JsonSetting(DEFAULT_SCL_PIN)},
    {"stepsPerRot", JsonSetting(DEFAULT_STEPS_PER_ROT)},
    {"maxVel", JsonSetting(DEFAULT_MAX_VEL)},
    {"charset", JsonSetting(DEFAULT_CHARSET)},
    {"transitionType", JsonSetting(DEFAULT_TRANSITION_TYPE)},
    // Operational States
    {"mode", JsonSetting(DEFAULT_MODE)}
});
// clang-format on

#include "modes/SingleInputMode.h"
#include "modes/MultiInputMode.h"
#include "modes/DateMode.h"
#include "modes/TimeMode.h"
#include "modes/RandomTestMode.h"
#include "modes/ManualMode.h"

WiFiClient wifiClient;
SplitFlapDisplay display(settings);
SplitFlapWebServer webServer(settings);
SplitFlapMqtt splitflapMqtt(settings, wifiClient);

// Mode instances
SingleInputMode singleInputMode(display, webServer, settings);
MultiInputMode multiInputMode(display, webServer, settings);
DateMode dateMode(display, webServer, settings);
TimeMode timeMode(display, webServer, settings);
RandomTestMode randomTestMode(display, webServer, settings);
ManualMode manualMode(display, webServer, settings);

DisplayMode* currentMode = nullptr;
int lastModeIndex = -1;

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
        webServer.setDisplay(&display);  // Connect display to web server for dynamic updates
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
        webServer.setDisplay(&display);  // Connect display to web server for dynamic updates
        splitflapMqtt.setup();
        splitflapMqtt.setDisplay(&display);
        splitflapMqtt.setWebServer(&webServer);  // Connect web server to MQTT for state updates
        display.setMqtt(&splitflapMqtt);

        display.homeToString("OK");
        delay(250);
        display.writeString("");
    }
}

void loop() {
    splitflapMqtt.loop();

    int modeIndex = webServer.getMode();
    
    // Switch mode if changed
    if (modeIndex != lastModeIndex) {
        if (currentMode) {
            currentMode->exit();
        }
        
        switch (modeIndex) {
            case 0: currentMode = &singleInputMode; break;
            case 1: currentMode = &multiInputMode; break;
            case 2: currentMode = &dateMode; break;
            case 3: currentMode = &timeMode; break;
            case 4: currentMode = nullptr; break; // Placeholder
            case 5: currentMode = &randomTestMode; break;
            case 6: currentMode = &manualMode; break;
            default: currentMode = nullptr; break;
        }
        
        if (currentMode) {
            currentMode->enter();
        }
        lastModeIndex = modeIndex;
    }

    if (currentMode) {
        currentMode->update();
    }

    webServer.handleOta();
    checkConnection();

    reconnectIfNeeded();

    webServer.checkRebootRequired();
    yield();
}

void checkConnection() {
    if (millis() - webServer.getLastCheckWifiTime() >
        webServer.getWifiCheckInterval()) { // check wifi to see if disconnected
        webServer.checkWiFi();
        webServer.setLastCheckWifiTime(millis());
    }
}

// Reconnection state variables
enum ReconnectState {
    RECONN_IDLE,
    RECONN_START,
    RECONN_WAIT_WIFI,
    RECONN_SHOW_OK,
    RECONN_CLEAR
};
ReconnectState reconnState = RECONN_IDLE;
unsigned long reconnTimer = 0;

void reconnectIfNeeded() {
    if (webServer.getAttemptReconnect() && reconnState == RECONN_IDLE) {
        webServer.setAttemptReconnect(false);
        reconnState = RECONN_START;
    }

    switch (reconnState) {
        case RECONN_IDLE:
            break;
            
        case RECONN_START:
            display.writeString("");
            if (! webServer.connectToWifi()) {
                webServer.startAccessPoint();
                webServer.enableOta();
                webServer.endMDNS();
                webServer.startMDNS();
                display.writeChar('X');
                reconnState = RECONN_IDLE; // Done (failed)
            } else {
                webServer.enableOta();
                webServer.endMDNS();
                webServer.startMDNS();
                display.writeString("OK");
                webServer.setWrittenString("OK");
                reconnTimer = millis();
                reconnState = RECONN_SHOW_OK;
            }
            splitflapMqtt.setup();
            break;
            
        case RECONN_SHOW_OK:
            if (millis() - reconnTimer > 500) {
                display.writeString("");
                webServer.setWrittenString("");
                reconnState = RECONN_IDLE; // Done (success)
            }
            break;
            
        default:
            reconnState = RECONN_IDLE;
            break;
    }
}

