// SplitFlapWebServer.h (header file)

#ifndef SplitFlapWebServer_h
#define SplitFlapWebServer_h
#include "Arduino.h"
#include "JsonSettings.h"
#include "LittleFS.h" // use `pio run -t uploadfs` to upload file system
#include "time.h" //for network time protocol
#include <ArduinoJson.h> //for settings json
#include <ESPAsyncWebServer.h> //by ESP32Async, Requires AsyncTCP by ESP32Async
#include <ESPmDNS.h>
#include <WiFi.h>

class SplitFlapWebServer {
  public:
    SplitFlapWebServer(JsonSettings &settings);
    void init();
    void setTimezone();

    // Wifi Connectivity
    bool loadWiFiCredentials();
    bool connectToWifi();
    bool getAttemptReconnect() const { return attemptReconnect; }
    void setAttemptReconnect(bool input) { attemptReconnect = input; }
    void startWebServer();
    void endMDNS();
    void startMDNS();
    void startAccessPoint();
    void checkWiFi();
    unsigned long getLastCheckWifiTime() { return lastCheckWifiTime; }
    void setLastCheckWifiTime(unsigned long input) { lastCheckWifiTime = input; }
    int getWifiCheckInterval() { return wifiCheckInterval; }

    // Mode
    int getMode();

    // Mode 0 - Single String
    String getInputString() const { return inputString; }
    String getWrittenString() const { return writtenString; }
    void setWrittenString(String input) { writtenString = input; }

    // Mode 1, Multi Input
    String getMultiInputString() const { return multiInputString; }
    int getMultiWordDelay() const { return multiWordDelay; }
    unsigned long getLastSwitchMultiTime() { return lastSwitchMultiTime; }
    void setLastSwitchMultiTime(unsigned long input) { lastSwitchMultiTime = input; }
    int getMultiWordCurrentIndex() { return multiWordCurrentIndex; }
    void setMultiWordCurrentIndex(int input) { multiWordCurrentIndex = input; }
    int getNumMultiWords() const { return numMultiWords; }

    // Mode 2, Date
    //  Function to get current minute as a string
    String getCurrentMinute();
    String getCurrentHour();
    String getDayPrefix(int n);
    String getMonthPrefix(int n);
    String getCurrentDay();
    unsigned long getLastCheckDateTime() { return lastCheckDateTime; }
    void setLastCheckDateTime(unsigned long input) { lastCheckDateTime = input; }
    int getDateCheckInterval() { return checkDateInterval; }

    int getCentering() { return centering; }

  private:
    JsonSettings &settings;

    String decodeURIComponent(String encodedString);
    void setInputString(String input) { inputString = input; }
    void setMultiInputString(String input) { multiInputString = input; }

    void setMode(int targetMode);
    void setMultiDelay(int input) { multiWordDelay = input; }

    unsigned long lastCheckDateTime;
    int checkDateInterval;

    int connectionMode; // 0 is AP mode, 1 is Internet Mode
    int centering; // whether to center text from custom imput

    int numMultiWords;
    unsigned long lastSwitchMultiTime;
    int multiWordDelay;
    int multiWordCurrentIndex;
    String multiInputString; // latest multi input from user

    String inputString; // latest single input from user
    String writtenString; // string for whatever is currently written to the display

    bool attemptReconnect;
    unsigned long lastCheckWifiTime;
    int wifiCheckInterval;
    AsyncWebServer server; // Declare server as a class member
};

#endif
//          __
// (QUACK)>(o )___
//          ( ._> /
//           `---'
// Morgan Manly
// 07/02/2025
