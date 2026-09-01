#pragma once

#include <Arduino.h>
#include <Wire.h>

class SplitFlapModule {
  public:
    // Constructor declarationS
    SplitFlapModule(); // default constructor required to allocate memory for
    // SplitFlapDisplay class
    SplitFlapModule(uint8_t I2Caddress, int stepsPerFullRotation, int stepOffset, int magnetPos, int charSetSize);

    void init();

    // Writes the idle pattern straight to an address, without needing a
    // constructed module. Used to kill the power-on coil current as early as
    // possible, before the settings are even loaded into modules.
    static void releaseCoils(uint8_t address);

    void step();                                             // advance the motor one full step
    void stop();                                             // write all motor input pins to low
    void start();                                            // re-energize coils in place, without moving

    int getMagnetPosition() const { return magnetPosition; } // position where magnet is detected
    int getCharPosition(char inputChar);                     // get integer position given single character
    int getPosition() const { return position; }             // get integer position
    int getCharsetSize() const { return numChars; }          // getter for charset size

    bool readHallEffectSensor();                             // return the value read by the hall effect
    // sensor
    void magnetDetected() {
        position = magnetPosition;
    } // update position to magnetposition, called when magnet is detected

    bool getHasErrored() const { return hasErrored; }

  private:
    uint8_t address;                    // i2c address of module
    int position;                       // character drum position
    int stepNumber;                     // index of the NEXT coil pattern to write
    int stepsPerRot;                    // number of steps per rotation
    bool hasErrored = false;            // set when the last i2c transaction failed, cleared when one succeeds

    void writeIO(uint16_t data);        // write to motor in pins

    int magnetPosition;                 // altered by offsets

    // Coil patterns for one electrical revolution of the 28BYJ-48 (2-phase-on
    // full stepping). Bits 1-4 drive the motor, bit 0 and bit 15 are left high:
    // bit 15 is the hall effect sensor input on the PCF8575.
    //   pattern 0 -> P01,P02   1 -> P01,P04   2 -> P03,P04   3 -> P02,P03
    // Writing them in increasing order turns the drum forwards; writing them in
    // decreasing order turns it backwards.
    static const uint16_t CoilStates[4];
    static const uint16_t IdleState; // all four coils low, hall input high

    const char *chars;               // pointer to active character set
    int charPositions[48];           // support up to 48 characters
    int numChars;                    // current number of characters
    int charSetSize;

    static const char StandardChars[37];
    static const char ExtendedChars[48];
};

// //PINs on the PCF8575 Board
// #define P00  	0
// #define P01  	1
// #define P02  	2
// #define P03  	3
// #define P04  	4
// #define P05  	5
// #define P06  	6
// #define P07  	7
// #define P10  	8
// #define P11  	9
// #define P12  	10
// #define P13  	11
// #define P14  	12
// #define P15  	13
// #define P16  	14
// #define P17  	15
