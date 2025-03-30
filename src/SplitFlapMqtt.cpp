#include "SplitFlapMqtt.h"

const char* mqtt_topic_command = "homeassistant/text/splitflap_display/set";
const char* mqtt_topic_state   = "homeassistant/text/splitflap_display/state";
const char* mqtt_topic_config  = "homeassistant/text/splitflap_display/config";

SplitFlapMqtt::SplitFlapMqtt(PubSubClient& client) : mqttClient(client) {}

void SplitFlapMqtt::setup(const String& server, int port, const String& user, const String& pass) {
    mqttServer = server;
    mqttPort = port;
    mqttUser = user;
    mqttPass = pass;
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
}

void SplitFlapMqtt::setMessageCallback(MQTT_CALLBACK_SIGNATURE) {
    mqttClient.setCallback(callback);
}

void SplitFlapMqtt::loop() {
    mqttClient.loop();
}

void SplitFlapMqtt::reconnect() {
    if (mqttClient.connected()) return;
    if (mqttServer == "") return;
    if (millis() - lastAttempt < 5000) return;
    if (retryCount > 10) return;

    lastAttempt = millis();
    retryCount++;

    bool connected = false;
    if (mqttUser != "" && mqttPass != "") {
        connected = mqttClient.connect("splitflap_display", mqttUser.c_str(), mqttPass.c_str(),
                                       "homeassistant/text/splitflap_display/availability", 0, true, "offline");
    } else {
        connected = mqttClient.connect("splitflap_display",
                                       "homeassistant/text/splitflap_display/availability", 0, true, "offline");
    }

    if (connected) {
        retryCount = 0;
        String payload = "{\"name\":\"Split Flap Display\",\"unique_id\":\"splitflap_display_01\",\"command_topic\":\"homeassistant/text/splitflap_display/set\",\"state_topic\":\"homeassistant/text/splitflap_display/state\"}";
        mqttClient.publish(mqtt_topic_config, payload.c_str(), true);
        mqttClient.subscribe(mqtt_topic_command);
        mqttClient.publish("homeassistant/text/splitflap_display/availability", "online", true);
        mqttClient.publish(mqtt_topic_state, "", true);
    }
}

void SplitFlapMqtt::publishState(const String& message) {
    mqttClient.publish(mqtt_topic_state, message.c_str(), true);
}