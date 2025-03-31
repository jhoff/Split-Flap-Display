#pragma once

#include "JsonSettings.h"
#include "SplitFlapDisplay.h"
#include <AsyncMqttClient.h>
#include <Ticker.h>

class SplitFlapMqtt {
public:
    SplitFlapMqtt(JsonSettings& settings);

    void setup();
    void publishState(const String& message);
    void setDisplay(SplitFlapDisplay* display);

private:
    AsyncMqttClient mqttClient;
    Ticker mqttReconnectTimer;
    JsonSettings& settings;
    SplitFlapDisplay* display;

    void connectToMqtt();
    static void staticReconnectCallback();
    static SplitFlapMqtt* instance;

    // MQTT config
    String mqttServer;
    int mqttPort = 1883;
    String mqttUser;
    String mqttPass;
    String mqtt_topic_command;
    String mqtt_topic_state;
    String mqtt_topic_config;
    String mqtt_topic_avail;

    // Internal reconnect tracking (optional)
    unsigned long lastAttempt = 0;
    int retryCount = 0;
};