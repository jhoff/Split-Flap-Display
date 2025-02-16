// Split Flap Display
// Morgan Manly
// 16/02/2025

// Enter SplitFlapDisplay.cpp to alter number of modules, and set addresses
// Enjoy :)

#include "Arduino.h" 
#include "SplitFlapDisplay.h"
#include "SplitFlapWebServer.h"

SplitFlapDisplay display; //Create Display Object
SplitFlapWebServer webServer; //Create Webserver Object

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Init Display");
  display.init(); //Initialise Display, and All Modules Within
  Serial.println("Init Web Server");
  webServer.init();

  if (!webServer.connectToWifi()) {
    webServer.startAccessPoint();
    webServer.startWebServer(); //Start Webserver

    if (display.getNumModules() == 8){
      display.homeToString("Wifi Err");
    }
    else{
      display.homeToChar('X');
    }
  }
  else{
    webServer.startWebServer(); //Start Webserver
    display.homeToString("OK");
    delay(250);
    display.writeString("");
  }
  
}

void loop() {

  // check what mode the display is in, this value is updated by the web server
  switch (webServer.getMode()) {
    case 0: { //single input mode
      singleInputMode();
      break;
    }
    case 1: { //multi input mode
      multiInputMode();
      break;
    }
    case 2: {//Date Mode
      dateMode();
      break;
    }
    case 3: {//Time Mode
      timeMode();
      break;
    }
    case 4: {//Count Mode
      break;
    }
    case 5: {//Random Test Mode
      randomTest();
      break;
    }
    default:
      break;
  } 

  checkConnection();

  reconnectIfNeeded();

  yield();
}

void singleInputMode(){
  String userInput = webServer.getInputString();
  if (userInput!=webServer.getWrittenString()){
    display.writeString(userInput,webServer.getCentering());
    webServer.setWrittenString(userInput);
  }
}

void multiInputMode() {
  if (millis() - webServer.getLastSwitchMultiTime() > webServer.getMultiWordDelay()) {
    webServer.setLastSwitchMultiTime(millis());
    //get user input, extract correct word from index using webserver counter, and display
    String userInput = webServer.getMultiInputString();
    String currWord = extractFromCSV(userInput,webServer.getMultiWordCurrentIndex());
    if (currWord!=webServer.getWrittenString()){
      display.writeString(currWord,webServer.getCentering());
      webServer.setWrittenString(currWord);
    }
    webServer.setMultiWordCurrentIndex((webServer.getMultiWordCurrentIndex()+1)%(webServer.getNumMultiWords()));
    
  }
}

void dateMode() {
  if (millis() - webServer.getLastCheckDateTime() > webServer.getDateCheckInterval()) {
    webServer.setLastCheckDateTime(millis());
    //get date and output to display
    String outputString = " ";
    switch(display.getNumModules()) {
      case 1: {
        break;
      }
      case 2: {
        outputString = webServer.getCurrentDay();
        break;
      }
      case 3: {
        outputString = webServer.getDayPrefix(3);
        break;
      }
      case 4: {
        outputString = " " + webServer.getCurrentDay() + " ";
        break;
      }
      case 5: {
        outputString = String(webServer.getDayPrefix(3)) + webServer.getCurrentDay();
        break;
      }
      case 6: {
        outputString = String(webServer.getDayPrefix(3)) + " " + webServer.getCurrentDay();
        break;
      }
      case 7: {
        outputString = String(webServer.getDayPrefix(3)) + "  " + webServer.getCurrentDay();
        break;
      }
      case 8: {
        outputString = String(webServer.getDayPrefix(3)) + webServer.getCurrentDay() + webServer.getMonthPrefix(3);
        break;
      }
    }

    if (outputString!=webServer.getWrittenString()){
      display.writeString(outputString,false,10);
      webServer.setWrittenString(outputString);
    }
  }
}

void timeMode() {
  if (millis() - webServer.getLastCheckDateTime() > webServer.getDateCheckInterval()) {
    webServer.setLastCheckDateTime(millis());
    //get time
    String outputString = " ";
    switch(display.getNumModules()) {
      case 1: {
        break;
      }
      case 2: {
        outputString = webServer.getCurrentMinute();
        break;
      }
      case 3: {
        outputString = " " + webServer.getCurrentMinute();
        break;
      }
      case 4: {
        outputString = webServer.getCurrentHour() + webServer.getCurrentMinute();
        break;
      }
      case 5: {
        outputString = webServer.getCurrentHour() +" "+ webServer.getCurrentMinute();
        break;
      }
      case 6: {
        outputString = " " + webServer.getCurrentHour() +" "+ webServer.getCurrentMinute();
        break;
      }
      case 7: {
        outputString = " " + webServer.getCurrentHour() +" "+ webServer.getCurrentMinute() + " ";
        break;
      }
      case 8: {
        outputString = "  " + webServer.getCurrentHour() + webServer.getCurrentMinute() + "  ";
        break;
      }
      default:
        break;
    }

    if (outputString!=webServer.getWrittenString()){
      display.writeString(outputString,false,10);
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
