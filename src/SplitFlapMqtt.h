#pragma once

#include "JsonSettings.h"
#include "SplitFlapDisplay.h"

#include <PubSubClient.h>
#include <WiFiClient.h>

// Forward declaration
class SplitFlapWebServer;

class SplitFlapMqtt {
  public:
    SplitFlapMqtt(JsonSettings &settings, WiFiClient &client); // updated constructor

    void setup();
    void loop();                                               // needed for PubSubClient3
    void publishState(const String &message);
    void setDisplay(SplitFlapDisplay *display);
    void setWebServer(SplitFlapWebServer *server);
    bool isConnected();
    void checkConnection();                                    // check and reconnect if needed

  private:
    PubSubClient mqttClient; // PubSubClient instead of AsyncMqttClient
    WiFiClient &wifiClient;  // store reference to WiFiClient

    JsonSettings &settings;
    SplitFlapDisplay *display;
    SplitFlapWebServer *webServer;

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

    // MQTT reconnection tracking
    unsigned long lastReconnectAttempt = 0;
    const unsigned long reconnectInterval = 5000;  // Try reconnecting every 5 seconds
    bool wasConnected = false;                     // Track previous connection state

    // Message queuing for busy display
    String pendingMessage = "";
    bool hasPendingMessage = false;
};
