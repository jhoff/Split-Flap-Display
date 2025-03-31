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
    String mqtt_topic_command;
    String mqtt_topic_state;
    String mqtt_topic_config;
    String mqtt_topic_avail;

    unsigned long lastAttempt = 0;
    int retryCount = 0;
};