
#include "Arduino.h" 
#include "SplitFlapDisplay.h"
#include "SplitFlapWebServer.h"


SplitFlapDisplay display; //Create Display Object
SplitFlapWebServer webServer; //Create Webserver Object

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  webServer.init();

  display.init(); //Initialise Display, and All Modules Within
  
  display.home(); //Home Display
  delay(500);

}

void loop() {

  
  // display.testAll();
  // display.testCount();
  // display.testRandom();
  delay(500);
  // display.write("Hey");
  // delay(500);
  // display.write("You");
  // delay(500);
  // display.write("Big");
  // delay(500);
  // display.write("Boy");
  // delay(500);
  // display.write("Sup");
  // delay(500);
  // display.write("Eat");
  // delay(500);
  // display.write("And");
  // delay(500);
  // display.write("Run");
  // delay(500);
  // display.write("now");
  // delay(500);
  // display.write("THX");
  // delay(500);
}
