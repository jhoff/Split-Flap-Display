#pragma once

#include "JsonSettings.h"
#include "SplitFlapDisplay.h"
#include <PubSubClient.h>
#include <WiFiClient.h>

class SplitFlapMqtt {
public:
    SplitFlapMqtt(JsonSettings& settings, WiFiClient& client); // updated constructor

    void setup();
    void loop(); // needed for PubSubClient3
    void publishState(const String& message);
    void setDisplay(SplitFlapDisplay* display);
    bool isConnected();

private:
    PubSubClient mqttClient;             // PubSubClient instead of AsyncMqttClient
    WiFiClient& wifiClient;              // store reference to WiFiClient

    JsonSettings& settings;
    SplitFlapDisplay* display;

    void connectToMqtt();

    // MQTT config
    String mqttServer;
    int mqttPort = 1883;
    String mqttUser;
    String mqttPass;
    String topic_command;
    String topic_state;
    String topic_avail;
    String topic_config_text;
    String topic_config_sensor;

    unsigned long lastAttempt = 0;
    int retryCount = 0;
};