#include "SplitFlapMqtt.h"

SplitFlapMqtt* SplitFlapMqtt::instance = nullptr;

SplitFlapMqtt::SplitFlapMqtt(JsonSettings& settings)
  : settings(settings), display(nullptr) {
    instance = this;
}

void SplitFlapMqtt::setup() {
    mqttServer = settings.getString("mqtt_server");
    mqttPort   = settings.getInt("mqtt_port");
    mqttUser   = settings.getString("mqtt_user");
    mqttPass   = settings.getString("mqtt_pass");

    String mdns = settings.getString("mdns");
    String name = settings.getString("name");
    mqtt_topic_command = "homeassistant/text/" + mdns + "/set";
    mqtt_topic_state   = "homeassistant/text/" + mdns + "/state";
    mqtt_topic_config  = "homeassistant/text/" + mdns + "/config";
    mqtt_topic_avail   = "homeassistant/text/" + mdns + "/availability";


    mqttClient.onConnect([this, mdns, name](bool sessionPresent) {
        Serial.println("[MQTT] Connected to broker");
        retryCount = 0;

        String payload = "{\"name\":\"" + name +"\",\"unique_id\":\"" + mdns +
                         "\",\"command_topic\":\"" + mqtt_topic_command +
                         "\",\"state_topic\":\"" + mqtt_topic_state + "\"}";

        mqttClient.subscribe(mqtt_topic_command.c_str(), 0);
        mqttClient.publish(mqtt_topic_avail.c_str(), 0, true, "online");
        mqttClient.publish(mqtt_topic_state.c_str(), 0, true, "");
        mqttClient.publish(mqtt_topic_config.c_str(), 0, true, payload.c_str());
    });

    mqttClient.onDisconnect([](AsyncMqttClientDisconnectReason reason) {
        Serial.println("[MQTT] Disconnected. Reconnecting...");
        if (SplitFlapMqtt::instance) {
            SplitFlapMqtt::instance->mqttReconnectTimer.once(5.0f, &SplitFlapMqtt::staticReconnectCallback);
        }
    });

    mqttClient.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties props, size_t len, size_t, size_t) {
        String message = String(payload).substring(0, len);
        Serial.printf("[MQTT] Message received: %s\n", message.c_str());
        if (display) {
            float maxVel = settings.getFloat("maxVel");
            display->writeString(message, maxVel, false);
            publishState(message);
        }
    });

    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    if (mqttUser.length() > 0) {
        mqttClient.setCredentials(mqttUser.c_str(), mqttPass.c_str());
    }

    connectToMqtt();
}

void SplitFlapMqtt::connectToMqtt() {
    Serial.println("[MQTT] Attempting to connect...");
    mqttClient.connect();
}

void SplitFlapMqtt::setDisplay(SplitFlapDisplay* d) {
    display = d;
}

void SplitFlapMqtt::publishState(const String& message) {
    Serial.println("[MQTT] Publishing state: " + message);
    mqttClient.publish(mqtt_topic_state.c_str(), 0, true, message.c_str());
}

void SplitFlapMqtt::staticReconnectCallback() {
    if (instance) instance->connectToMqtt();
}