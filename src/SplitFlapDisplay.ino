// Split Flap Display
// Morgan Manly 02/16/2025
// Jordan Hoff 03/25/2025
// Thom Koopman 03/30/2025

// Enjoy :)
#include "Arduino.h"
#include "SplitFlapDisplay.h"
#include "SplitFlapWebServer.h"
#include "SplitFlapMqtt.h"
#include "JsonSettings.h"

JsonSettings settings = JsonSettings("config", {
    // General Settings
    {"name", JsonSetting("My Display")},
    {"mdns", JsonSetting("splitflap")},
    {"timezone", JsonSetting("Etc/UTC")},
    // Wifi Settings
    {"ssid", JsonSetting("")},
    {"password", JsonSetting("")},
    // MQTT Settings
    {"mqtt_server", JsonSetting("")},
    {"mqtt_port", JsonSetting(1883)},
    {"mqtt_user", JsonSetting("")},
    {"mqtt_pass", JsonSetting("")},
    // Hardware Settings
    {"moduleCount", JsonSetting(8)},
    {"moduleAddresses", JsonSetting({0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27})},
    {"magnetPosition", JsonSetting(730)},
    {"moduleOffsets", JsonSetting({0,0,0,0,0,0,0,0})},
    {"displayOffset", JsonSetting(0)},
    {"sdaPin", JsonSetting(8)},
    {"sclPin", JsonSetting(9)},
    {"stepsPerRot", JsonSetting(2048)},
    {"maxVel", JsonSetting(15.0f)},
    // Operational States
    {"mode", JsonSetting(0)}
});

SplitFlapDisplay display(settings);
SplitFlapWebServer webServer(settings);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
SplitFlapMqtt splitflapMqtt(mqttClient, settings);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(SERIAL_SPEED);

  #ifdef STARTUP_DELAY
    delay(STARTUP_DELAY);
  #endif

  Serial.println("Init Web Server");
  webServer.init();

  if (! webServer.connectToWifi()) {
    webServer.startAccessPoint();
    webServer.startMDNS();
    webServer.startWebServer();

    display.init();
    display.homeToString("");

    if (display.getNumModules() == 8){
      display.writeString("Wifi Err");
    }
    else{
      display.writeChar('X');
    }
  }
  else{
    webServer.startMDNS();
    webServer.startWebServer();

    display.init();
    splitflapMqtt.setup();
    splitflapMqtt.setDisplay(&display);
    display.homeToString("");

    display.writeString("OK");
    delay(250);
    display.writeString("");
  }
}

void loop() {
  splitflapMqtt.loop();

  // check what mode the display is in, this value is updated by the web server
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
  if (userInput!=webServer.getWrittenString()){
    display.writeString(userInput,MAX_RPM,webServer.getCentering());
    webServer.setWrittenString(userInput);
  }
}

void multiInputMode() {
  if (millis() - webServer.getLastSwitchMultiTime() > webServer.getMultiWordDelay()) {
    //get user input, extract correct word from index using webserver counter, and display
    String userInput = webServer.getMultiInputString();
    String currWord = extractFromCSV(userInput,webServer.getMultiWordCurrentIndex());
    if (currWord!=webServer.getWrittenString()){
      display.writeString(currWord,MAX_RPM,webServer.getCentering());
      webServer.setWrittenString(currWord);
    }
    webServer.setLastSwitchMultiTime(millis());
    webServer.setMultiWordCurrentIndex((webServer.getMultiWordCurrentIndex()+1)%(webServer.getNumMultiWords()));
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
      display.homeToString(outputString, settings.getFloat("maxVel"));
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
      display.writeString(outputString, settings.getFloat("maxVel"), false);
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
    if (! webServer.connectToWifi()) {
      webServer.startAccessPoint();
      webServer.endMDNS();
      webServer.startMDNS();
      display.writeChar('X');
    } else {
      webServer.endMDNS();
      webServer.startMDNS();
      display.writeString("OK");
      webServer.setWrittenString("OK");
      delay(500);
      display.writeString("");
      webServer.setWrittenString("");
    }
    Serial.println("[MQTT] 2");
    splitflapMqtt.setup();
    splitflapMqtt.reconnect();
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

//          __
// (QUACK)>(o )___
//          ( ._> /
//           `---'
