#pragma once

#include "JsonSettings.h"
#include "SplitFlapModule.h"

#include <Arduino.h>

#define MAX_MODULES 8 // for memory allocation, update if more modules
#define MAX_RPM 15.0f

// Timing constants for motor control
#define HALL_EFFECT_CHECK_INTERVAL_US  (20 * 1000)  // 20ms minimum to avoid sensor bouncing
#define MOTOR_START_STOP_DELAY_MS      200          // Time for motor to align to magnetic field
#define WATCHDOG_FEED_INTERVAL_MS      100          // Feed watchdog every 100ms during operations

class SplitFlapMqtt;

/**
 * @brief Main controller class for the Split Flap Display.
 * 
 * Manages multiple SplitFlapModule instances, handles high-level display operations
 * (writing strings, homing), and coordinates with MQTT.
 */
class SplitFlapDisplay {
  public:
    SplitFlapDisplay(JsonSettings &settings);

    /**
     * @brief Initialize the display and all modules.
     */
    void init();

    /**
     * @brief Update module offsets from settings without full re-initialization.
     */
    void updateOffsets();

    /**
     * @brief Write a string to the display.
     * 
     * @param inputString The string to display
     * @param speed Motor speed in RPM
     * @param centering Whether to center the text if shorter than display width
     */
    void writeString(
        String inputString, float speed = MAX_RPM,
        bool centering = true
    );

    /**
     * @brief Set all modules to a single character.
     * 
     * @param inputChar The character to display
     * @param speed Motor speed in RPM
     */
    void writeChar(char inputChar,
                   float speed = MAX_RPM);

    /**
     * @brief Move modules to specific step positions.
     * 
     * @param targetPositions Array of target positions for each module
     * @param speed Motor speed in RPM
     * @param releaseMotors Whether to de-energize motors after movement
     * @param isHoming Whether this movement is part of a homing sequence
     */
    void moveTo(int targetPositions[], float speed = MAX_RPM, bool releaseMotors = true, bool isHoming = false);

    /**
     * @brief Home all modules (find zero position).
     * 
     * @param speed Motor speed in RPM
     */
    void home(float speed = MAX_RPM);

    /**
     * @brief Home all modules and then display a string.
     * 
     * @param homeString The string to display after homing
     * @param speed Motor speed in RPM
     * @param centering Whether to center the text
     */
    void homeToString(
        String homeString, float speed = MAX_RPM,
        bool centering = true
    );

    /**
     * @brief Home all modules and then display a character.
     * 
     * @param homeChar The character to display after homing
     * @param speed Motor speed in RPM
     */
    void homeToChar(char homeChar,
                    float speed = MAX_RPM);

    void testAll();
    void testCount();
    void testRandom(float speed = MAX_RPM);

    /**
     * @brief Test a single module (cycle A -> 0 -> blank).
     * 
     * @param moduleIndex Index of the module to test
     * @param speed Motor speed in RPM
     */
    void testModule(int moduleIndex, float speed = MAX_RPM);

    int getNumModules() { return numModules; }
    int getCharsetSize() const { return charSetSize; }
    void setMqtt(SplitFlapMqtt *mqttHandler);
    SplitFlapModule* getModules() { return modules; }       // Get access to modules array for testing
    bool isBusy() const { return isMoving; }                // Check if display is currently moving

  private:
    JsonSettings &settings;

    bool checkAllFalse(bool array[], int size);
    void stopMotors();
    void startMotors();
    void performHomingSequence(float speed);  // Shared homing logic

    int numModules;
    uint8_t moduleAddresses[MAX_MODULES];
    SplitFlapModule modules[MAX_MODULES];
    int moduleOffsets[MAX_MODULES];
    int displayOffset;

    float maxVel;       // Max Velocity In RPM
    int charSetSize;    // 37 for standard, 48 for extended
    int stepsPerRot;    // number of motor steps per full rotation of character
                        // drum
    int magnetPosition; // position of drum wheel when magnet is detected
    int SDAPin;         // SDA pin
    int SCLPin;         // SCL pin

    bool isMoving = false;  // Flag to indicate if display is currently moving
    SplitFlapMqtt *mqtt = nullptr;
};
