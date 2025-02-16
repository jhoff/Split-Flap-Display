// SplitFlapDisplay.h (header file)
#ifndef SplitFlapDisplay_h
#define SplitFlapDisplay_h

#include <Arduino.h>
#include "SplitFlapModule.h"

#define MAX_MODULES 8 //for memory allocation, update if more modules

class SplitFlapDisplay {
  public:

    SplitFlapDisplay(); //Constructor
    void init();
    void writeString(String inputString,bool centering=true,float speed=15); //Move all modules at once to show a specific string
    void writeChar(char inputChar); //sets all modules to a single char
    void moveTo(int targetPositions[], float speed = 15, bool releaseMotors = true);
    void home(); //move home
    void homeToString(String homeString); //moves home and then writes a string
    void homeToChar(char homeChar); //moves home and then sets all modules to a char
    void testAll();
    void testCount();
    void testRandom();
    int getNumModules() { return numModules; }
    
  private:
    bool checkAllFalse(bool array[], int size);
    void stopMotors();
    void startMotors();

    static const int numModules;             // Number of modules in the display 
    SplitFlapModule modules[MAX_MODULES];   // Array of SplitFlapModule objects, size of array is maximum number of modules allowed in class
    static const int moduleOffsets[MAX_MODULES];
    static const uint8_t moduleAddresses[MAX_MODULES];

    static const float maxVel; //Max Velocity In RPM
    float maxStepsPerSecond; //calculated from maxVel
    static const int stepsPerRotation; //number of motor steps per full rotation of character drum
    static const int magnetPosition; //position of drum wheel when magnet is detected
    static const int SDAPin;      // SDA pin
    static const int SCLPin;      // SCL pin
  
};

#endif
//          __
// (QUACK)>(o )___
//          ( ._> /
//           `---'
// Morgan Manly
// 28/01/2025