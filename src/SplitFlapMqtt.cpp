#include "SplitFlapMqtt.h"
#include "SplitFlapWebServer.h"

SplitFlapMqtt::SplitFlapMqtt(JsonSettings &settings, WiFiClient &wifiClient)
    : settings(settings), wifiClient(wifiClient), mqttClient(wifiClient), display(nullptr), webServer(nullptr) {}

void SplitFlapMqtt::setup() {
    mqttServer = settings.getString("mqtt_server");
    mqttPort = settings.getInt("mqtt_port");
    mqttUser = settings.getString("mqtt_user");
    mqttPass = settings.getString("mqtt_pass");

    String mdns = settings.getString("mdns");
    String name = settings.getString("name");

    topic_command = "splitflap/" + mdns + "/set";
    topic_state = "splitflap/" + mdns + "/state";
    topic_avail = "splitflap/" + mdns + "/availability";
    topic_config_text = "homeassistant/text/splitflap_text_" + mdns + "/config";
    topic_config_sensor = "homeassistant/sensor/splitflap_sensor_" + mdns + "/config";

    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length) {
        String message;
        for (unsigned int i = 0; i < length; i++) {
            message += (char) payload[i];
        }
        Serial.printf("[MQTT] Message received: %s\n", message.c_str());
        if (display) {
            // Check if display is busy
            if (display->isBusy()) {
                Serial.println("[MQTT] Display busy, queuing message");
                pendingMessage = message;
                hasPendingMessage = true;
            } else {
                float maxVel = settings.getFloat("maxVel");
                display->writeString(message, maxVel, false);
                // Update the web server's state to prevent mode logic from overwriting
                if (webServer) {
                    webServer->setInputString(message);      // Update input to match
                    webServer->setWrittenString(message);    // Update written to match
                }
            }
        }
    });

    connectToMqtt();
}

void SplitFlapMqtt::connectToMqtt() {
    if (! mqttClient.connected()) {
        Serial.println("[MQTT] Attempting to connect...");
        String mdns = settings.getString("mdns");
        String name = settings.getString("name");

        if (mqttUser.length() > 0) {
            mqttClient.connect(mdns.c_str(), mqttUser.c_str(), mqttPass.c_str());
        } else {
            mqttClient.connect(mdns.c_str());
        }

        if (mqttClient.connected()) {
            Serial.println("[MQTT] Connected to broker");

            // clang-format off
            String payload_text = "{"
                "\"name\":\"Display\","
                "\"unique_id\":\"text_" + mdns + "\","
                "\"command_topic\":\"" + topic_command + "\","
                "\"availability_topic\":\"" + topic_avail + "\","
                "\"device\":{"
                    "\"identifiers\":[\"splitflap_" + mdns + "\"],"
                    "\"name\":\"" + name + "\","
                    "\"manufacturer\":\"SplitFlap\","
                    "\"model\":\"SplitFlap Display\","
                    "\"sw_version\":\"1.0.0\""
                "}"
            "}";

            String payload_sensor = "{"
                "\"name\":\"Currently Displayed\","
                "\"unique_id\":\"sensor_" + mdns + "\","
                "\"state_topic\":\"" + topic_state + "\","
                "\"availability_topic\":\"" + topic_avail + "\","
                "\"entity_category\":\"diagnostic\","
                "\"device\":{"
                    "\"identifiers\":[\"splitflap_" + mdns + "\"],"
                    "\"name\":\"" + name + "\","
                    "\"manufacturer\":\"SplitFlap\","
                    "\"model\":\"SplitFlap Display\","
                    "\"sw_version\":\"1.0.0\""
                "}"
            "}";
            // clang-format on

            mqttClient.subscribe(topic_command.c_str());
            mqttClient.publish(topic_avail.c_str(), "online", true);
            mqttClient.publish(topic_state.c_str(), "", true);

            mqttClient.publish(topic_config_text.c_str(), payload_text.c_str(), true);
            mqttClient.publish(topic_config_sensor.c_str(), payload_sensor.c_str(), true);
        } else {
            Serial.println("[MQTT] Failed to connect");
        }
    }
}

void SplitFlapMqtt::setDisplay(SplitFlapDisplay *d) {
    display = d;
}

void SplitFlapMqtt::setWebServer(SplitFlapWebServer *ws) {
    webServer = ws;
}

void SplitFlapMqtt::publishState(const String &message) {
    Serial.println("[MQTT] Publishing state: " + message);
    mqttClient.publish(topic_state.c_str(), message.c_str(), true);
}

void SplitFlapMqtt::loop() {
    mqttClient.loop();
    checkConnection();  // Check and reconnect if needed

    // Process pending message if display is no longer busy
    if (hasPendingMessage && display && !display->isBusy()) {
        Serial.printf("[MQTT] Processing queued message: %s\n", pendingMessage.c_str());
        float maxVel = settings.getFloat("maxVel");
        display->writeString(pendingMessage, maxVel, false);
        // Update the web server's state to prevent mode logic from overwriting
        if (webServer) {
            webServer->setInputString(pendingMessage);
            webServer->setWrittenString(pendingMessage);
        }
        hasPendingMessage = false;
        pendingMessage = "";
    }
}

void SplitFlapMqtt::checkConnection() {
    bool currentlyConnected = mqttClient.connected();

    // Check if MQTT server is configured
    if (mqttServer.length() == 0) {
        return; // No MQTT server configured, skip reconnection
    }

    // Detect state change from connected to disconnected
    if (wasConnected && !currentlyConnected) {
        Serial.println("[MQTT] Connection lost!");
        // Publish offline status before losing connection completely
        mqttClient.publish(topic_avail.c_str(), "offline", true);
    }

    // Attempt reconnection if disconnected
    if (!currentlyConnected) {
        unsigned long now = millis();

        // Check if enough time has passed since last attempt
        if (now - lastReconnectAttempt >= reconnectInterval) {
            lastReconnectAttempt = now;
            Serial.println("[MQTT] Attempting to reconnect...");
            connectToMqtt();
        }
    } else {
        // Connected - detect state change from disconnected to connected
        if (!wasConnected) {
            Serial.println("[MQTT] Connection established/restored!");
        }
    }

    wasConnected = currentlyConnected;
}

bool SplitFlapMqtt::isConnected() {
    return mqttClient.connected();
}
