#include "SplitFlapDisplay.h"

#include "JsonSettings.h"
#include "SplitFlapModule.h"
#include "SplitFlapMqtt.h"
#include <esp_task_wdt.h>

SplitFlapDisplay::SplitFlapDisplay(JsonSettings &settings) : settings(settings) {}

void SplitFlapDisplay::init() {
    numModules = settings.getInt("moduleCount");
    stepsPerRot = settings.getInt("stepsPerRot");
    displayOffset = settings.getInt("displayOffset");
    magnetPosition = settings.getInt("magnetPosition");
    maxVel = settings.getFloat("maxVel");
    charSetSize = settings.getInt("charset");

    std::vector<int> settingAddresses = settings.getIntVector("moduleAddresses");
    for (int i = 0; i < numModules; i++) {
        moduleAddresses[i] = (uint8_t) settingAddresses[i];
    }

    std::vector<int> settingOffsets = settings.getIntVector("moduleOffsets");
    for (int i = 0; i < numModules; i++) {
        moduleOffsets[i] = settingOffsets[i];
    }

    Serial.print("Module Offsets: ");
    for (int i = 0; i < numModules; i++) {
        Serial.print(moduleOffsets[i]);
        Serial.print(" ");
    }
    Serial.println();

    for (uint8_t i = 0; i < numModules; i++) {
        modules[i] = SplitFlapModule(
            moduleAddresses[i], stepsPerRot, moduleOffsets[i] + displayOffset, magnetPosition, charSetSize
        );
    }

    SDAPin = settings.getInt("sdaPin");
    SCLPin = settings.getInt("sclPin");

    Wire.begin(SDAPin, SCLPin);
    Wire.setClock(400000);

    for (uint8_t i = 0; i < numModules; i++) {
        modules[i].init();
    }
}

void SplitFlapDisplay::updateOffsets() {
    // Reload offsets from settings
    displayOffset = settings.getInt("displayOffset");
    
    std::vector<int> settingOffsets = settings.getIntVector("moduleOffsets");
    for (int i = 0; i < numModules; i++) {
        moduleOffsets[i] = settingOffsets[i];
        // Update each module's offset
        modules[i].updateOffset(moduleOffsets[i] + displayOffset);
    }
    
    Serial.println("Module offsets updated dynamically");
    Serial.print("Display Offset: ");
    Serial.println(displayOffset);
    Serial.print("Module Offsets: ");
    for (int i = 0; i < numModules; i++) {
        Serial.print(moduleOffsets[i]);
        Serial.print(" ");
    }
    Serial.println();
}

void SplitFlapDisplay::testAll() {
    char testChars[37] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                          'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    int numChars = sizeof(testChars) / sizeof(testChars[0]);
    int targetPositions[numModules];

    int charPos;
    for (int i = 0; i < numChars; i++) {
        // Serial.print("Target Positions: [");
        // fill array with same char

        for (int j = 0; j < numModules; j++) {
            targetPositions[j] = modules[j].getCharPosition(testChars[i]);
            // Serial.print(targetPositions[j]);
            // Serial.print(" , ");
        }
        // Serial.println("]");

        moveTo(targetPositions);
        delay(500);
    }
}

void SplitFlapDisplay::testRandom(float speed) {
    char testChars[37] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                          'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    int targetPositions[numModules];
    char randChar;

    Serial.print("Target: ");
    for (int i = 0; i < numModules; i++) {
        randChar = testChars[random(0, 37)];
        targetPositions[i] = modules[i].getCharPosition(randChar);
        Serial.print(randChar);
    }
    Serial.println(" ");
    moveTo(targetPositions, speed);
}

void SplitFlapDisplay::testModule(int moduleIndex, float speed) {
    if (moduleIndex < 0 || moduleIndex >= numModules) {
        Serial.println("Invalid module index");
        return;
    }

    Serial.print("Homing module ");
    Serial.println(moduleIndex);

    int targetPositions[numModules];

    // Get current positions for all modules (keep them stationary)
    for (int i = 0; i < numModules; i++) {
        targetPositions[i] = modules[i].getPosition();
    }

    // Set target for the specific module to trigger homing
    // Move back one step to ensure we cross the magnet sensor
    targetPositions[moduleIndex] = (modules[moduleIndex].getPosition() - 1 + stepsPerRot) % stepsPerRot;

    startMotors();
    moveTo(targetPositions, speed, true, true);  // isHoming = true

    Serial.print("Module ");
    Serial.print(moduleIndex);
    Serial.print(" homed to position: ");
    Serial.println(modules[moduleIndex].getPosition());

    // Move to blank space to show alignment
    // Use smaller delays with yields to avoid watchdog timeout
    for (int i = 0; i < 5; i++) {
        delay(100);
        yield();  // Allow other tasks to run
    }

    Serial.println("Moving to blank to show alignment");
    for (int i = 0; i < numModules; i++) {
        targetPositions[i] = modules[i].getPosition();
    }
    targetPositions[moduleIndex] = modules[moduleIndex].getCharPosition(' ');
    moveTo(targetPositions, speed);

    Serial.print("Module at position: ");
    Serial.print(modules[moduleIndex].getPosition());
    Serial.println(" (should show blank)");
}

void SplitFlapDisplay::testCount() {
    int count = 0;
    int maxCount = pow(10, numModules);
    char targetChar;
    int targetInteger;

    int targetPositions[numModules];

    for (int i = 0; i < maxCount; i++) {
        // get each character in the count integer
        for (int j = 0; j < numModules; j++) {
            targetInteger = (i % (int) pow(10, j + 1)) / (int) pow(10, j);
            targetChar = targetInteger + '0'; // convert to char
            targetPositions[numModules - j - 1] = modules[j].getCharPosition(targetChar);
        }

        moveTo(targetPositions);
        delay(250);
    }
}

// Private helper method: Perform the homing sequence
void SplitFlapDisplay::performHomingSequence(float speed) {
    Serial.println("Homing");
    Serial.print("Initial positions: ");
    for (int i = 0; i < numModules; i++) {
        Serial.print(modules[i].getPosition());
        Serial.print(" ");
    }
    Serial.println();

    int targetPositions[numModules];
    for (int i = 0; i < numModules; i++) {
        targetPositions[i] = (modules[i].getPosition() - 1 + stepsPerRot) % stepsPerRot;
    }
    startMotors();
    moveTo(targetPositions, speed, false, true);  // isHoming = true

    Serial.print("Positions after magnet detection: ");
    for (int i = 0; i < numModules; i++) {
        Serial.print(modules[i].getPosition());
        Serial.print(" ");
    }
    Serial.println();
}

void SplitFlapDisplay::home(float speed) {
    performHomingSequence(speed);

    // Move to blank space
    int targetPositions[numModules];
    for (int i = 0; i < numModules; i++) {
        targetPositions[i] = modules[i].getCharPosition(' ');
    }

    Serial.print("Target positions for blank: ");
    for (int i = 0; i < numModules; i++) {
        Serial.print(targetPositions[i]);
        Serial.print(" ");
    }
    Serial.println();

    moveTo(targetPositions, speed);

    Serial.print("Final positions: ");
    for (int i = 0; i < numModules; i++) {
        Serial.print(modules[i].getPosition());
        Serial.print(" ");
    }
    Serial.println();
}

void SplitFlapDisplay::homeToString(String homeString, float speed, bool centering) {
    performHomingSequence(speed);
    writeString(homeString, speed, centering);
}

void SplitFlapDisplay::homeToChar(char homeChar, float speed) {
    performHomingSequence(speed);
    writeChar(homeChar, speed);
}

void SplitFlapDisplay::writeChar(char inputChar, float speed) {
    int targetPositions[numModules];
    // Iterate through the input string and process each character
    for (int i = 0; i < numModules; i++) {
        targetPositions[i] = modules[i].getCharPosition(inputChar);
    }
    moveTo(targetPositions, speed);
}

void SplitFlapDisplay::writeString(String inputString, float speed, bool centering) {
    String displayString = inputString.substring(0, numModules);

    if (centering) {
        int totalPadding = numModules - displayString.length();
        int paddingLeft = totalPadding / 2;
        int paddingRight = totalPadding - paddingLeft;

        // Add padding to the left
        String result = "";
        for (int i = 0; i < paddingLeft; i++) {
            result += " ";
        }

        // Add the original string
        result += displayString;

        // Add padding to the right
        for (int i = 0; i < paddingRight; i++) {
            result += " ";
        }
        displayString = result;
    } else {                                          // pad blanks to end, if no centering
        while (displayString.length() < numModules) { // Pad with spaces
            displayString += " ";                     // Padding with space
        }
    }

    int targetPositions[numModules];

    // Initialize all positions to blank space first
    for (int i = 0; i < numModules; i++) {
        targetPositions[i] = modules[i].getCharPosition(' ');
    }

    // Then set positions for the actual characters in the string
    for (int i = 0; i < displayString.length() && i < numModules; i++) {
        char currentChar = displayString[i];
        // Serial.println(currentChar);
        targetPositions[i] = modules[i].getCharPosition(currentChar);
    }
    moveTo(targetPositions, speed);

    if (mqtt && mqtt->isConnected()) {
        mqtt->publishState(displayString);
    }
}

void SplitFlapDisplay::moveTo(int targetPositions[], float speed, bool releaseMotors, bool isHoming) {
    // Validate input parameters
    if (targetPositions == nullptr) {
        Serial.println("ERROR: targetPositions is null, aborting moveTo");
        return;
    }

    // Validate all target positions are within valid range
    for (int i = 0; i < numModules; i++) {
        if (targetPositions[i] < 0 || targetPositions[i] >= stepsPerRot) {
            Serial.printf("ERROR: Module %d target position %d out of range (0-%d), aborting moveTo\n",
                         i, targetPositions[i], stepsPerRot - 1);
            return;
        }
    }

    isMoving = true;  // Set busy flag at start of movement

    speed = constrain(speed, 2, maxVel);
    float stepsPerSecond = (speed / 60) * stepsPerRot;
    float timePerStep = 1000000 / stepsPerSecond;

    unsigned long currentTime = micros();

    // Use predefined timing constants for reliable operation

    bool resetLatches[numModules] = {}; // Initialize to false //start with latch on to prevent case where the
    // motion starts with the magnet over the sensor
    bool needsStepping[numModules] = {};             // Initialize to false; //modules that still require moving
    unsigned long lastStepTimes[numModules] = {};    // Initialize to false; //track when each module was last stepped
    unsigned long lastSensorCheckTime = currentTime; // track when we last read all the hall effect sensors
    bool sensorTriggered[numModules] = {};           // Track which modules triggered their hall sensor

    for (int i = 0; i < numModules; i++) {
        targetPositions[i] = constrain(
            targetPositions[i],
            0,
            stepsPerRot - 1
        ); // Constrain to avoid errors with incorrect inputs
        resetLatches[i] = true;
        lastStepTimes[i] = currentTime;
        if (modules[i].getPosition() != targetPositions[i]) {
            needsStepping[i] = true;
        } else {
            needsStepping[i] = false;
        }
    }

    // Wake up all motors before starting movement
    // This gentle sequence ensures coils are energized and overcomes static friction
    for (int i = 0; i < numModules; i++) {
        modules[i].wakeUp();
    }

    startMotors(); // not sure if this helps or not, likely that it does not based
    // on testing
    delay(MOTOR_START_STOP_DELAY_MS); // give the motor time to align to magnetic field

    bool isFinished = checkAllFalse(needsStepping, numModules);
    unsigned long lastWatchdogFeed = millis();

    // Get transition type
    int transitionType = settings.getInt("transitionType");
    unsigned long startDelayMicros[numModules];
    unsigned long movementStartTime = micros();

    if (transitionType == 1) { // Synchronized Landing
        int maxDistance = 0;
        int travelDistances[numModules];

        // Calculate travel distances
        for (int i = 0; i < numModules; i++) {
            int currentPos = modules[i].getPosition();
            int targetPos = targetPositions[i];
            
            if (targetPos >= currentPos) {
                travelDistances[i] = targetPos - currentPos;
            } else {
                travelDistances[i] = (stepsPerRot - currentPos) + targetPos;
            }
            
            if (travelDistances[i] > maxDistance) {
                maxDistance = travelDistances[i];
            }
        }

        // Calculate start delays
        for (int i = 0; i < numModules; i++) {
            // delay = (max steps - my steps) * time per step
            startDelayMicros[i] = (unsigned long)((maxDistance - travelDistances[i]) * timePerStep);
        }
    } else {
        // Normal mode - no delays
        for (int i = 0; i < numModules; i++) {
            startDelayMicros[i] = 0;
        }
    }

    while (! isFinished) {
        currentTime = micros();

        // Feed the watchdog timer periodically to prevent timeout
        if (millis() - lastWatchdogFeed > WATCHDOG_FEED_INTERVAL_MS) {
            yield();
            esp_task_wdt_reset();  // Reset the task watchdog timer
            lastWatchdogFeed = millis();
        }

        bool anySteppedOrStarted = false;

        for (int i = 0; i < numModules; i++) {
            // Check synchronization delay
             if (transitionType == 1) {
                 if (currentTime - movementStartTime < startDelayMicros[i]) {
                    continue; // Wait for our synchronized start time
                 }
            }
            
            anySteppedOrStarted = true;

            if (((currentTime - lastStepTimes[i]) > timePerStep) && needsStepping[i]) {
                modules[i].step();
                lastStepTimes[i] = currentTime;  // Use consistent time reference
                if (modules[i].getPosition() == targetPositions[i]) { // this module is not in the correct position,
                    // requires stepping
                    needsStepping[i] = false;
                }
            }
        }
        
        if (!anySteppedOrStarted) {
            delay(1); 
        }

        if ((currentTime - lastSensorCheckTime) > HALL_EFFECT_CHECK_INTERVAL_US) { // check hall effect sensor every checkIntervalMs
            // check every modules sensor
            for (int i = 0; i < numModules; i++) {
                if (needsStepping[i] &&
                    (modules[i].readHallEffectSensor() == true
                    )) { // only check sensors where the module is still moving
                    if (! resetLatches[i]) {
                        // Track that this module's sensor was triggered (for debug summary)
                        sensorTriggered[i] = true;

                        // UNCOMMENTING THIS WILL PROBBALY MAKE THE MOTORS INACCURATE, DUE
                        // TO TIME TAKEN TO PRINT
                        //  Serial.print("Module: ");
                        //  Serial.print(i);
                        //  Serial.print(" Magnet Position: ");
                        //  Serial.print(modules[i].getMagnetPosition());
                        //  Serial.print(" Actual Position: ");
                        //  Serial.print(modules[i].getPosition());
                        //  Serial.print(" Error: ");
                        //  Serial.println((modules[i].getMagnetPosition() -
                        //  modules[i].getPosition()));
                        modules[i].magnetDetected(); // update position to the modules
                        // magnet position
                        resetLatches[i] = true;
                    }
                } else if (resetLatches[i] == true) {
                    resetLatches[i] = false;
                }
            }
            isFinished = checkAllFalse(needsStepping, numModules);
            lastSensorCheckTime = micros(); // Get fresh timestamp after sensor checks
        }
    }
    if (releaseMotors) {
        delay(MOTOR_START_STOP_DELAY_MS); // allow all motors time to settle
        stopMotors();
    }

    // Print hall sensor summary if this is a homing operation
    if (isHoming) {
        Serial.print("Hall sensor summary - Triggered modules: ");
        bool anyTriggered = false;
        for (int i = 0; i < numModules; i++) {
            if (sensorTriggered[i]) {
                if (anyTriggered) {
                    Serial.print(", ");
                }
                Serial.print(i);
                anyTriggered = true;
            }
        }
        if (!anyTriggered) {
            Serial.print("None");
        }
        Serial.println();
    }

    isMoving = false;  // Clear busy flag at end of movement
}

bool SplitFlapDisplay::checkAllFalse(bool array[], int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == true) {
            return false;              // As soon as a true value is found, return false
        }
    }
    return true;                       // All values were false
}

void SplitFlapDisplay::startMotors() { // Probably broken somewhere, not sure
    // why, haven't looked
    for (int i = 0; i < numModules; i++) {
        modules[i].start();
    }
}

void SplitFlapDisplay::stopMotors() {
    // Serial.println("Stopping Motors");
    for (int i = 0; i < numModules; i++) {
        modules[i].stop();
    }
}

void SplitFlapDisplay::setMqtt(SplitFlapMqtt *mqttHandler) {
    mqtt = mqttHandler;
}
