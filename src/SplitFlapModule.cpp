#include "SplitFlapModule.h"

// Array of characters, in order, the first item is located on the magnet on the
// character drum
const char SplitFlapModule::StandardChars[37] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
                                                 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
                                                 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

const char SplitFlapModule::ExtendedChars[48] = {
    ' ', 'A', 'B', 'C', 'D', 'E',  'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U',  'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '\'', ':', '?', '!', '.', '-', '/', '$', '@', '#', '%',
};

bool hasErrored = false;

// Default Constructor
SplitFlapModule::SplitFlapModule() : SplitFlapModule(0x20, 2048, 0, 710, 37, String()) {
    magnetPosition = 710;
}

// Constructor implementation
SplitFlapModule::SplitFlapModule(
    uint8_t I2Caddress, int stepsPerFullRotation, int stepOffset, int magnetPos, int charsetSize,
    const String &charsetStr
)
    : address(I2Caddress), position(0), stepNumber(0), stepsPerRot(stepsPerFullRotation) {
    magnetPosition = magnetPos + stepOffset;

    int len = charsetStr.length();

    if (len < 37) {
        // Use StandardChars as fallback
        Serial.println("Fallback StandardChars");
        charSetSize = numChars = 37;
        usingCustomChars = false;
        for (int i = 0; i < numChars; i++) {
            customChars[i] = StandardChars[i];
        }
    } else if (len >= 37) {
        // Use custom charset, but truncate to either 37 or 48
        usingCustomChars = true;
        charSetSize = numChars = (len >= 48) ? 48 : 37;
        for (int i = 0; i < numChars; i++) {
            customChars[i] = charsetStr[i];
        }
        customChars[numChars] = '\0';
    } else {
        // Fallback if empty
        charSetSize = numChars = 37;
        usingCustomChars = false;
        for (int i = 0; i < numChars; i++) {
            customChars[i] = StandardChars[i];
        }
    }
}

void SplitFlapModule::writeIO(uint16_t data) {
    Wire.beginTransmission(address);
    Wire.write(data & 0xFF);        // Send lower byte
    Wire.write((data >> 8) & 0xFF); // Send upper byte

    byte error = Wire.endTransmission();

    if (error > 0 && ! hasErrored) {
        hasErrored = true; // Set the error flag
        Serial.print("Error writing data to module ");
        Serial.print(address);
        Serial.print(", error code: ");
        Serial.println(error); // Error codes:
        // 0 = success
        // 1 = data too long to fit in transmit buffer
        // 2 = received NACK on transmit of address
        // 3 = received NACK on transmit of data
        // 4 = other error
    }
}

// Init Module, Setup IO Board
void SplitFlapModule::init() {
    float stepSize = (float) stepsPerRot / (float) numChars;
    float currentPosition = 0;
    for (int i = 0; i < numChars; i++) {
        charPositions[i] = (int) currentPosition;
        currentPosition += stepSize;
    }

    uint16_t initState = 0b1111111111100001; // Pin 15 (17) as INPUT, Pins 1-4 as OUTPUT
    writeIO(initState);

    stop();                                  // Write all motor coil inputs LOW

    int initDelay = 100;

    delay(initDelay);
    step();
    delay(initDelay);
    step();
    delay(initDelay);
    step();
    delay(initDelay);
    step();
    delay(initDelay);

    stop();
}

int SplitFlapModule::getCharPosition(char inputChar) {
    inputChar = toupper(inputChar);

    for (int i = 0; i < charSetSize; i++) {
        if (customChars[i] == inputChar) {
            return charPositions[i];
        }
    }

    return 0; // fallback
}

void SplitFlapModule::stop() {
    uint16_t stepState = 0b1111111111100001;
    writeIO(stepState);
}

void SplitFlapModule::start() {
    stepNumber = (stepNumber + 3) % 4; // effectively take one off stepNumber
    step(false);                       // write the "previous" step high again, in case turned off
}

void SplitFlapModule::step(bool updatePosition) {
    uint16_t stepState;
    switch (stepNumber) {
        case 0:
            stepState = 0b1111111111100111;
            writeIO(stepState);
            break;
        case 1:
            stepState = 0b1111111111110011;
            writeIO(stepState);
            break;
        case 2:
            stepState = 0b1111111111111001;
            writeIO(stepState);
            break;
        case 3:
            stepState = 0b1111111111101101;
            writeIO(stepState);
            break;
    }
    if (updatePosition) {
        position = (position + 1) % stepsPerRot;
        stepNumber = (stepNumber + 1) % 4;
    }
}

bool SplitFlapModule::readHallEffectSensor() {
    if (hasErrored) {
        return false;
    }

    uint8_t requestBytes = 2;
    Wire.requestFrom(address, requestBytes);
    // Make sure the data is available
    if (Wire.available() == 2) {
        uint16_t inputState = 0;

        // Read the two bytes and combine them into a 16-bit value
        inputState = Wire.read();             // Read the lower byte
        inputState |= (Wire.read() << 8);     // Read the upper byte and shift it left

        return (inputState & (1 << 15)) != 0; // If bit is 15, return HIGH, else LOW
    }
    return false;
}
