#include "SplitFlapMqtt.h"

SplitFlapMqtt::SplitFlapMqtt(JsonSettings& settings, WiFiClient& wifiClient)
  : settings(settings), wifiClient(wifiClient), mqttClient(wifiClient), display(nullptr) {}

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

    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        String message;
        for (unsigned int i = 0; i < length; i++) {
            message += (char)payload[i];
        }
        Serial.printf("[MQTT] Message received: %s\n", message.c_str());
        if (display) {
            float maxVel = settings.getFloat("maxVel");
            display->writeString(message, maxVel, false);
        }
    });

    connectToMqtt();
}

void SplitFlapMqtt::connectToMqtt() {
    if (!mqttClient.connected()) {
        Serial.println("[MQTT] Attempting to connect...");
        String clientId = "SplitFlap-" + settings.getString("mdns");
        if (mqttUser.length() > 0) {
            mqttClient.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str());
        } else {
            mqttClient.connect(clientId.c_str());
        }

        if (mqttClient.connected()) {
            Serial.println("[MQTT] Connected to broker");

        String payload =
            "{"
                "\"name\":\"" + settings.getString("name") + "\","
                "\"unique_id\":\"" + settings.getString("mdns") + "\","
                "\"object_id\":\"" + settings.getString("mdns") + "\","
                "\"command_topic\":\"" + mqtt_topic_command + "\","
                "\"state_topic\":\"" + mqtt_topic_state + "\","
                "\"availability_topic\":\"" + mqtt_topic_avail + "\","
                "\"device\":{"
                    "\"identifiers\":[\"" + settings.getString("mdns") + "\"],"
                    "\"name\":\"" + settings.getString("name") + "\","
                    "\"manufacturer\":\"SplitFlap\","
                    "\"model\":\"SplitFlap Display \","
                    "\"sw_version\":\"1.0.0\""
                "}"
            "}";

            mqttClient.subscribe(mqtt_topic_command.c_str());
            mqttClient.publish(mqtt_topic_avail.c_str(), "online", true);
            mqttClient.publish(mqtt_topic_state.c_str(), "", true);
            mqttClient.publish(mqtt_topic_config.c_str(), payload.c_str(), true);
        } else {
            Serial.println("[MQTT] Failed to connect");
        }
    }
}

void SplitFlapMqtt::setDisplay(SplitFlapDisplay* d) {
    display = d;
}

void SplitFlapMqtt::publishState(const String& message) {
    Serial.println("[MQTT] Publishing state: " + message);
    mqttClient.publish(mqtt_topic_state.c_str(), message.c_str(), true);
}

void SplitFlapMqtt::loop() {
    mqttClient.loop();
}

bool SplitFlapMqtt::isConnected() {
  return mqttClient.connected();
}