// SplitFlapDisplay.h (header file)
#ifndef SplitFlapDisplay_h
#define SplitFlapDisplay_h

#include "JsonSettings.h"
#include "SplitFlapModule.h"
#include <Arduino.h>

#define MAX_MODULES 8 // for memory allocation, update if more modules
#define MAX_RPM 15.0f

class SplitFlapMqtt;

class SplitFlapDisplay {
public:
  SplitFlapDisplay(JsonSettings &settings);

  void init();
  void
  writeString(String inputString, float speed = MAX_RPM,
              bool centering =
                  true); // Move all modules at once to show a specific string
  void writeChar(char inputChar,
                 float speed = MAX_RPM); // sets all modules to a single char
  void moveTo(int targetPositions[], float speed = MAX_RPM,
              bool releaseMotors = true);
  void home(float speed = MAX_RPM); // move home
  void
  homeToString(String homeString, float speed = MAX_RPM,
               bool centering = true); // moves home and then writes a string
  void homeToChar(
      char homeChar,
      float speed = MAX_RPM); // moves home and then sets all modules to a char
  void testAll();
  void testCount();
  void testRandom(float speed = MAX_RPM);
  int getNumModules() { return numModules; }
  void setMqtt(SplitFlapMqtt *mqttHandler);

private:
  JsonSettings &settings;

  bool checkAllFalse(bool array[], int size);
  void stopMotors();
  void startMotors();

  int numModules;
  uint8_t moduleAddresses[MAX_MODULES];
  SplitFlapModule modules[MAX_MODULES];
  int moduleOffsets[MAX_MODULES];
  int displayOffset;

  float maxVel;       // Max Velocity In RPM
  int stepsPerRot;    // number of motor steps per full rotation of character
                      // drum
  int magnetPosition; // position of drum wheel when magnet is detected
  int SDAPin;         // SDA pin
  int SCLPin;         // SCL pin

  SplitFlapMqtt *mqtt = nullptr;
};

#endif
//          __
// (QUACK)>(o )___
//          ( ._> /
//           `---'
// Morgan Manly
// 28/01/2025
