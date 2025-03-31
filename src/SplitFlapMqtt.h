#pragma once

#include <PubSubClient.h>
#include "JsonSettings.h"
#include "SplitFlapDisplay.h"

class SplitFlapMqtt {
public:
    SplitFlapMqtt(PubSubClient& client, JsonSettings& settings);

    void setup();
    void loop();
    void reconnect();
    void publishState(const String& message);
    void setDisplay(SplitFlapDisplay* display);
    void publishDisplayContent(const String& message);

private:
    PubSubClient& mqttClient;
    JsonSettings& settings;
    SplitFlapDisplay* display = nullptr;

    String mqttServer;
    int mqttPort;
    String mqttUser;
    String mqttPass;

    unsigned long lastAttempt = 0;
    int retryCount = 0;

    void handleMessage(char* topic, byte* payload, unsigned int length);
};