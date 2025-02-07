// SplitFlapModule_h.cpp (implementation file)c:\Users\morga\Documents\Arduino\libraries\PCF8575_library\PCF8575_library.h
#include "SplitFlapModule.h"
#include <cctype> // Required for tolower()

// Array of characters, in order, the first item is located on the magnet on the character drum
const char SplitFlapModule::chars[37] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const int SplitFlapModule::numChars = sizeof(SplitFlapModule::chars) / sizeof(SplitFlapModule::chars[0]);
int SplitFlapModule::charPositions[37]; //to be written in init function

// Not used but useful to have
// const int SplitFlapModule::motorPins[] = {P06, P05, P04, P03}; // List of pins to set as OUTPUT
// const int SplitFlapModule::HallEffectPIN = P17; //Input pin for Hall effect sensor

//Default Constructor
SplitFlapModule::SplitFlapModule()
    : address(0), position(0), stepNumber(0),stepsPerRotation(0){ //default values
  magnetPosition = 700;
}

// Constructor implementation
SplitFlapModule::SplitFlapModule(uint8_t I2Caddress,int stepsPerFullRotation, int stepOffset)
    : address(I2Caddress), position(0), stepNumber(0), stepsPerRotation(stepsPerFullRotation){
  magnetPosition = 710 + stepOffset;
}

void SplitFlapModule::writeIO(uint16_t data) {
  Wire.beginTransmission(address);
  Wire.write(data & 0xFF);       // Send lower byte
  Wire.write((data >> 8) & 0xFF); // Send upper byte

  byte error = Wire.endTransmission();
  if (error == 0) {
    // Serial.println("Data written successfully.");
  } else {
    // Serial.print("Error writing data, error code: ");
    // Serial.println(error);  // Error codes:
    //                         // 0 = success
    //                         // 1 = data too long to fit in transmit buffer
    //                         // 2 = received NACK on transmit of address
    //                         // 3 = received NACK on transmit of data
    //                         // 4 = other error
  }
}

//Init Module, Setup IO Board
void SplitFlapModule::init() {

  // uint16_t initState = 0b1111111110000111; // Pin 15 (17) as INPUT, Pins 3-6 as OUTPUT

  uint16_t initState = 0b1111111111100001; // Pin 15 (17) as INPUT, Pins 1-4 as OUTPUT
  writeIO(initState);

  stop(); //Write all motor coil inputs LOW
  
  //Generate Character Position Array
  float stepSize = (float)stepsPerRotation / (float)numChars;
  float currentPosition = 0;
  for (int i = 0; i < numChars; i++) {
      charPositions[i] = (int)currentPosition;
      currentPosition += stepSize;
  }
}

int SplitFlapModule::getCharPosition(char inputChar) {
    inputChar = toupper(inputChar);
    for (int i = 0; i < numChars; i++) {
        if (chars[i] == inputChar) {
            return charPositions[i];
        }
    }
    return -1;  // Character not found
}

void SplitFlapModule::stop() {
  // IO.digitalWrite(motorPins[0], LOW);   
  // IO.digitalWrite(motorPins[1], LOW);   
  // IO.digitalWrite(motorPins[2], LOW);   
  // IO.digitalWrite(motorPins[3], LOW);   
  uint16_t stepState = 0b1111111111100001;
  writeIO(stepState);
}   

void SplitFlapModule::start() {
  stepNumber = (stepNumber + 3) % 4; //effectively take one off stepNumber
  step(false); //write the "previous" step high again, in case turned off
}

void SplitFlapModule::step(bool updatePosition) {
  uint16_t stepState;
  switch(stepNumber){
    case 0:
      // IO.digitalWrite(motorPins[0], LOW);   
      // IO.digitalWrite(motorPins[1], LOW);   
      // IO.digitalWrite(motorPins[2], HIGH);   
      // IO.digitalWrite(motorPins[3], HIGH);   
      // stepState = 0b1111111110011111;
      stepState = 0b1111111111100111;
      writeIO(stepState);
      break;
    case 1:
      // IO.digitalWrite(motorPins[0],HIGH);
      // IO.digitalWrite(motorPins[1],LOW);
      // IO.digitalWrite(motorPins[2],LOW);
      // IO.digitalWrite(motorPins[3],HIGH);
      // stepState = 0b1111111111001111;
      stepState = 0b1111111111110011;
      writeIO(stepState);
      break;
    case 2:
      // IO.digitalWrite(motorPins[0], HIGH);   
      // IO.digitalWrite(motorPins[1], HIGH);   
      // IO.digitalWrite(motorPins[2], LOW);   
      // IO.digitalWrite(motorPins[3], LOW); 
      // stepState = 0b1111111111100111;
      stepState = 0b1111111111111001;
      writeIO(stepState);
      break;
    case 3:
      // IO.digitalWrite(motorPins[0], LOW);   
      // IO.digitalWrite(motorPins[1], HIGH);   
      // IO.digitalWrite(motorPins[2], HIGH);   
      // IO.digitalWrite(motorPins[3], LOW);   
      // stepState = 0b1111111110110111;
      stepState = 0b1111111111101101;
      writeIO(stepState);
      break;
  }
  if (updatePosition){
    position = (position + 1) % stepsPerRotation;
    stepNumber = (stepNumber + 1) % 4;
  }
}


bool SplitFlapModule::readHallEffectSensor(){

  uint8_t requestBytes = 2;
  Wire.requestFrom(address, requestBytes);
  // Make sure the data is available
  if (Wire.available() == 2) {
    uint16_t inputState = 0;
    
    // Read the two bytes and combine them into a 16-bit value
    inputState = Wire.read();            // Read the lower byte
    inputState |= (Wire.read() << 8);    // Read the upper byte and shift it left

    return (inputState & (1 << 15)) != 0; // If bit is 15, return HIGH, else LOW

  }
  return false;
  // return IO.digitalRead(HallEffectPIN);
}




