#pragma once

#include <Arduino.h>
#include <Wire.h>

// PCF8575 I/O Expander Pin Configuration
// Bits 0-3: Stepper motor control pins (outputs)
// Bit 15: Hall effect sensor input
#define PCF8575_IO_INIT_STATE     0b1111111111100001  // Pin 15 input, pins 0-3 output
#define PCF8575_MOTOR_STOP_STATE  0b1111111111100001  // All motor pins LOW

// Stepper Motor Control Patterns (4-step sequence)
// These patterns energize different coil combinations for smooth stepping
#define STEPPER_PATTERN_0  0b1111111111100111  // Coils A+B
#define STEPPER_PATTERN_1  0b1111111111110011  // Coils B+C
#define STEPPER_PATTERN_2  0b1111111111111001  // Coils C+D
#define STEPPER_PATTERN_3  0b1111111111101101  // Coils D+A

// Initialization timing
#define MODULE_INIT_DELAY_MS  100  // Delay between initialization steps

/**
 * @brief Controls a single split-flap display module.
 * 
 * Handles motor control, position tracking, and sensor reading for one character module.
 * Uses a PCF8575 I/O expander for motor control.
 */
class SplitFlapModule {
  public:
    // Constructor declarations
    SplitFlapModule(); // default constructor required to allocate memory for
    // SplitFlapDisplay class
    
    /**
     * @brief Construct a new Split Flap Module object
     * 
     * @param I2Caddress I2C address of the PCF8575
     * @param stepsPerFullRotation Steps required for a full rotation
     * @param stepOffset Offset steps from magnet position
     * @param magnetPos Base magnet position
     * @param charSetSize Number of characters in the set (37 or 48)
     */
    SplitFlapModule(uint8_t I2Caddress, int stepsPerFullRotation, int stepOffset, int magnetPos, int charSetSize);

    /**
     * @brief Initialize the module and energize motor coils.
     */
    void init();

    /**
     * @brief Update the step offset dynamically.
     * 
     * @param newOffset New offset value in steps
     */
    void updateOffset(int newOffset);

    /**
     * @brief Perform a single motor step.
     * 
     * @param updatePosition Whether to update the internal position counter
     */
    void step(bool updatePosition = true);

    /**
     * @brief Stop the motor (de-energize coils).
     */
    void stop();

    /**
     * @brief Start the motor (energize coils to current step).
     */
    void start();

    /**
     * @brief Perform a gentle wake-up sequence to overcome static friction.
     */
    void wakeUp();

    int getMagnetPosition() const { return magnetPosition; } // position where magnet is detected
    
    /**
     * @brief Get the step position for a specific character.
     * 
     * @param inputChar The character to find
     * @return int Step position of the character
     */
    int getCharPosition(char inputChar);
    
    int getPosition() const { return position; }             // get integer position
    int getCharsetSize() const { return numChars; }          // getter for charset size

    /**
     * @brief Read the Hall effect sensor state.
     * 
     * @return true if magnet detected, false otherwise
     */
    bool readHallEffectSensor();

    /**
     * @brief Update position to magnet position (called when magnet detected).
     */
    void magnetDetected() {
        position = magnetPosition;
    }

    bool getHasErrored() const { return hasErrored; }

    /**
     * @brief Test if the module responds on the I2C bus.
     * 
     * @return true if responsive, false otherwise
     */
    bool testI2CConnectivity();
    
    uint8_t getAddress() const { return address; }           // get I2C address

  private:
    uint8_t address;                // i2c address of module
    int position;                   // character drum position
    int stepNumber;                 // current position in the stepping order, to make motor move
    int stepsPerRot;                // number of steps per rotation
    bool hasErrored = false;        // flag to indicate if an error has occurred

    // Motor reliability tracking
    int consecutiveErrors = 0;           // track I2C communication errors

    void writeIO(uint16_t data);    // write to motor in pins

    int magnetPosition;             // altered by offsets
    int baseMagnetPosition;         // original magnet position before offset
    static const int motorPins[];   // Array of motor pins
    static const int HallEffectPIN; // Hall Effect Sensor Pin (On PCF8575)

    const char *chars;              // pointer to active character set
    int charPositions[48];          // support up to 48 characters
    int numChars;                   // current number of characters
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
