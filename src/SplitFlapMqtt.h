#pragma once
#include <PubSubClient.h>
extern const char* mqtt_topic_command;
extern const char* mqtt_topic_state;
extern const char* mqtt_topic_config;

class SplitFlapMqtt {
public:
    SplitFlapMqtt(PubSubClient& client);
    void setup(const String& server, int port, const String& user, const String& pass);
    void loop();
    void reconnect();
    void publishState(const String& message);
    void setMessageCallback(MQTT_CALLBACK_SIGNATURE);

private:
    PubSubClient& mqttClient;
    String mqttServer;
    String mqttUser;
    String mqttPass;
    int mqttPort;
    unsigned long lastAttempt = 0;
    int retryCount = 0;
};