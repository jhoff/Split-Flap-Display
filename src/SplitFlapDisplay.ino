// Split Flap Display
// Morgan Manly
// 16/02/2025

// Edit config.dist.h to alter number of modules, and set addresses
// Enjoy :)

#include "Arduino.h"
#include "SplitFlapDisplay.h"
#include "SplitFlapWebServer.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define MQTT_MAX_PACKET_SIZE 512

SplitFlapDisplay display; //Create Display Object
SplitFlapWebServer webServer; //Create Webserver Object
WiFiClient espClient;
PubSubClient mqttClient(espClient);
Preferences prefs;

String ssid, password, mqtt_server, mqtt_user, mqtt_pass;
int mqtt_port;

const char* mqtt_topic_command = "homeassistant/text/splitflap_display/set";
const char* mqtt_topic_state   = "homeassistant/text/splitflap_display/state";
const char* mqtt_topic_config  = "homeassistant/text/splitflap_display/config";

void reconnectMQTT();
void handleMqttMessage(char* topic, byte* payload, unsigned int length);

String currentDisplayText = "";

void setup() {
  Serial.begin(SERIAL_SPEED);

  prefs.begin("wifi", true);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");
  prefs.end();

  prefs.begin("mqtt", true);
  mqtt_server = prefs.getString("mqtt_server", "");
  mqtt_port = prefs.getInt("mqtt_port", 8883);
  mqtt_user = prefs.getString("mqtt_user", "");
  mqtt_pass = prefs.getString("mqtt_pass", "");
  prefs.end();

  Serial.println("Init Display");
  display.init(); //Initialise Display, and All Modules Within

  display.homeToString("");

  Serial.println("Init Web Server");
  webServer.init();

  static AsyncWebServer server(80);

  #ifdef STARTUP_DELAY
    delay(STARTUP_DELAY);
  #endif

  if (!webServer.connectToWifi()) {
    webServer.startAccessPoint();
    webServer.startWebServer(); //Start Webserver

    if (display.getNumModules() == 8){
      display.writeString("Wifi Err");
    }
    else{
      display.writeChar('X');
    }
  }
  else{
    webServer.startWebServer(); //Start Webserver
    //display.writeString("OK");
    delay(250);
    //display.writeString("");

    mqttClient.setServer(mqtt_server.c_str(), mqtt_port);
    mqttClient.setCallback(handleMqttMessage);
    reconnectMQTT();
  }
}

void loop() {
  mqttClient.loop();

  switch (webServer.getMode()) {
    case 0: singleInputMode(); break;
    case 1: multiInputMode(); break;
    case 2: dateMode(); break;
    case 3: timeMode(); break;
    case 4: break;
    case 5: randomTest(); break;
    default: break;
  }

  checkConnection();
  reconnectIfNeeded();
  yield();
}

void singleInputMode(){
  String userInput = webServer.getInputString();
  if (userInput != webServer.getWrittenString()) {
    display.writeString(userInput, MAX_RPM, webServer.getCentering());
    webServer.setWrittenString(userInput);
  }
}

void multiInputMode() {
  if (millis() - webServer.getLastSwitchMultiTime() > webServer.getMultiWordDelay()) {
    //get user input, extract correct word from index using webserver counter, and display
    String userInput = webServer.getMultiInputString();
    String currWord = extractFromCSV(userInput, webServer.getMultiWordCurrentIndex());
    if (currWord != webServer.getWrittenString()) {
      display.writeString(currWord, MAX_RPM, webServer.getCentering());
      webServer.setWrittenString(currWord);
    }
    webServer.setLastSwitchMultiTime(millis());
    webServer.setMultiWordCurrentIndex((webServer.getMultiWordCurrentIndex() + 1) % (webServer.getNumMultiWords()));
  }
}

void dateMode() {
  if (millis() - webServer.getLastCheckDateTime() > webServer.getDateCheckInterval()) {
    webServer.setLastCheckDateTime(millis());
    String outputString = " ";
    switch (display.getNumModules()) {
      case 2: outputString = webServer.getCurrentDay(); break;
      case 3: outputString = webServer.getDayPrefix(3); break;
      case 4: outputString = " " + webServer.getCurrentDay() + " "; break;
      case 5: outputString = webServer.getDayPrefix(3) + webServer.getCurrentDay(); break;
      case 6: outputString = webServer.getDayPrefix(3) + " " + webServer.getCurrentDay(); break;
      case 7: outputString = webServer.getDayPrefix(3) + "  " + webServer.getCurrentDay(); break;
      case 8: outputString = webServer.getDayPrefix(3) + webServer.getCurrentDay() + webServer.getMonthPrefix(3); break;
      default: break;
    }
    if (outputString != webServer.getWrittenString()) {
      display.homeToString(outputString, MAX_RPM);
      webServer.setWrittenString(outputString);
    }
  }
}

void timeMode() {
  if (millis() - webServer.getLastCheckDateTime() > webServer.getDateCheckInterval()) {
    webServer.setLastCheckDateTime(millis());
    String outputString = " ";
    switch (display.getNumModules()) {
      case 2: outputString = webServer.getCurrentMinute(); break;
      case 3: outputString = " " + webServer.getCurrentMinute(); break;
      case 4: outputString = webServer.getCurrentHour() + webServer.getCurrentMinute(); break;
      case 5: outputString = webServer.getCurrentHour() + " " + webServer.getCurrentMinute(); break;
      case 6: outputString = " " + webServer.getCurrentHour() + " " + webServer.getCurrentMinute(); break;
      case 7: outputString = " " + webServer.getCurrentHour() + " " + webServer.getCurrentMinute() + " "; break;
      case 8: outputString = "  " + webServer.getCurrentHour() + webServer.getCurrentMinute() + "  "; break;
      default: break;
    }
    if (outputString != webServer.getWrittenString()) {
      display.writeString(outputString, MAX_RPM, false);
      webServer.setWrittenString(outputString);
    }
  }
}

void randomTest(){
  display.testRandom();
  delay(2500);
}

void checkConnection() {
  if (millis()-webServer.getLastCheckWifiTime() > webServer.getWifiCheckInterval()){ //check wifi to see if disconnected
    webServer.checkWiFi();
    webServer.setLastCheckWifiTime(millis());
  }
}

void reconnectIfNeeded(){
  if (webServer.getAttemptReconnect()) { //check if the device should attempt reconnection to wifi
    webServer.setAttemptReconnect(false);
    display.writeString("");
    if (!webServer.connectToWifi()) {
      webServer.startAccessPoint();
      display.writeChar('X');
    }
    else{
      display.writeString("OK");
      webServer.setWrittenString("OK");
      delay(500);
      display.writeString("");
      webServer.setWrittenString("");
    }
  }
  if (webServer.isMqttReconnectRequired()) {
    Serial.println("MQTT settings changed — reconnecting...");

    prefs.begin("mqtt", true);
    mqtt_server = prefs.getString("mqtt_server", "");
    mqtt_port = prefs.getInt("mqtt_port", 1883);
    mqtt_user = prefs.getString("mqtt_user", "");
    mqtt_pass = prefs.getString("mqtt_pass", "");
    prefs.end();

    reconnectMQTT();

    webServer.clearMqttReconnectFlag();
  }
}

String extractFromCSV(String str, int index) {
    int startIndex = 0;
    int endIndex = str.length();

    int commaCount = 0;
    for (int i = 0; i < str.length(); i++) {
      if (str[i] == ','){
        commaCount++;
        if (commaCount == index){
          startIndex = i+1; //skip past the comma
        }
        else if (commaCount == index+1){
          endIndex = i;
        }
      }
    }

    return str.substring(startIndex, endIndex);
}

void reconnectMQTT() {
  static unsigned long lastAttempt = 0;
  static int retryCount = 0;
    bool connected;

  if (mqttClient.connected()) return;
  if (mqtt_server == "") {
    Serial.println("MQTT server not configured. Skipping connection.");
    return;
  }
  if (millis() - lastAttempt < 5000) return; // retry every 5s max
  if (retryCount > 10) return; // give up after 10 tries

  lastAttempt = millis();
  retryCount++;

  Serial.print("Connecting to MQTT...");

  if (mqtt_user != "" && mqtt_pass != "") {
    connected = mqttClient.connect("splitflap_display", mqtt_user.c_str(), mqtt_pass.c_str(),
                                   "homeassistant/text/splitflap_display/availability", 0, true, "offline");
  } else {
    connected = mqttClient.connect("splitflap_display",
                                   "homeassistant/text/splitflap_display/availability", 0, true, "offline");
  }

  if (connected) {
    Serial.println("connected");
    retryCount = 0;
    String discoveryTopic = "homeassistant/text/splitflap_display/config";
    String discoveryPayload = "{\"name\":\"Split Flap\",\"unique_id\":\"splitflap_display_01\",\"command_topic\":\"homeassistant/text/splitflap_display/set\",\"state_topic\":\"homeassistant/text/splitflap_display/state\"}";

    bool success = mqttClient.publish(discoveryTopic.c_str(), discoveryPayload.c_str(), true);
    if (success) {
      Serial.println("Mqtt discovery successful");
      mqttClient.subscribe(mqtt_topic_command);
      mqttClient.publish("homeassistant/text/splitflap_display/availability", "online", true);
      mqttClient.publish(mqtt_topic_state, "", true);
    } else {
      Serial.println("Mqtt discovery failed");
    }
  } else {
    Serial.print(" failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void handleMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("MQTT message received: ");
  Serial.println(message);

  display.writeString(message, MAX_RPM, true);
  mqttClient.publish(mqtt_topic_state, message.c_str(), true);
}

//          __
// (QUACK)>(o )___
//          ( ._> /
//           `---'
