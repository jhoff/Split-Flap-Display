#include "SplitFlapMqtt.h"

const char* mqtt_topic_command = "homeassistant/text/splitflap_display/set";
const char* mqtt_topic_state   = "homeassistant/text/splitflap_display/state";
const char* mqtt_topic_config  = "homeassistant/text/splitflap_display/config";
const char* mqtt_topic_avail   = "homeassistant/text/splitflap_display/availability";

SplitFlapMqtt::SplitFlapMqtt(PubSubClient& client, JsonSettings& settings)
  : mqttClient(client), settings(settings), display(nullptr), lastAttempt(0), retryCount(0) {}

void SplitFlapMqtt::setup() {
    mqttServer = settings.getString("mqtt_server");
    mqttPort   = settings.getInt("mqtt_port");
    mqttUser   = settings.getString("mqtt_user");
    mqttPass   = settings.getString("mqtt_pass");

    Serial.println("[MQTT] Setup with:");
    Serial.println("Server: " + mqttServer);
    Serial.println("Port: " + String(mqttPort));
    Serial.println("User: " + mqttUser);

    mqttClient.setServer(mqttServer.c_str(), mqttPort);

    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->handleMessage(topic, payload, length);
    });

    reconnect();
}

void SplitFlapMqtt::setDisplay(SplitFlapDisplay* d) {
    display = d;
}

void SplitFlapMqtt::loop() {
    mqttClient.loop();
}

void SplitFlapMqtt::reconnect() {
    if (mqttClient.connected()) return;
    if (mqttServer == "") {
        Serial.println("[MQTT] No MQTT server configured, skipping reconnect");
        return;
    }

    if (millis() - lastAttempt < 5000) return;
    if (retryCount > 10) {
        Serial.println("[MQTT] Too many failed attempts, skipping reconnect");
        return;
    }

    lastAttempt = millis();
    retryCount++;

    bool connected = false;
    if (mqttUser != "" && mqttPass != "") {
        connected = mqttClient.connect("splitflap_display", mqttUser.c_str(), mqttPass.c_str(),
                                       mqtt_topic_avail, 0, true, "offline");
    } else {
        connected = mqttClient.connect("splitflap_display", mqtt_topic_avail, 0, true, "offline");
    }

    if (connected) {
        Serial.println("[MQTT] Connected to broker!");
        retryCount = 0;

        String payload = "{\"name\":\"Split Flap Display\",\"unique_id\":\"splitflap_display_01\",\"command_topic\":\""
                         + String(mqtt_topic_command) + "\",\"state_topic\":\"" + String(mqtt_topic_state) + "\"}";
        mqttClient.publish(mqtt_topic_config, payload.c_str(), true);
        Serial.println("[MQTT] Published discovery config");

        mqttClient.subscribe(mqtt_topic_command);
        Serial.println("[MQTT] Subscribed to command topic");

        mqttClient.publish(mqtt_topic_avail, "online", true);
        mqttClient.publish(mqtt_topic_state, "", true);
        Serial.println("[MQTT] Published availability and cleared state");
    } else {
        Serial.println("[MQTT] Connection failed");
    }
}

void SplitFlapMqtt::publishState(const String& message) {
    Serial.println("[MQTT] Publishing state: " + message);
    mqttClient.publish(mqtt_topic_state, message.c_str(), true);
}

void SplitFlapMqtt::handleMessage(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("[MQTT] Message received: ");
    Serial.println(message);

    if (display) {
        float maxVel = settings.getFloat("maxVel");
        display->writeString(message, maxVel, true);
    } else {
        Serial.println("[MQTT] Display pointer is null, cannot write message.");
    }

    publishState(message);
}