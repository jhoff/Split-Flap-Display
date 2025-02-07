#include "SplitFlapDisplay.h"
#include "SplitFlapModule.h"


const int SplitFlapDisplay::numModules = 3;
const uint8_t SplitFlapDisplay::moduleAddresses[] = {0x20,0x21,0x22,0x23,0x24,0x25}; // Module I2C Addresses, In Order
// const uint8_t SplitFlapDisplay::moduleAddresses[] = {0x20,0x24}; // Module I2C Addresses, In Order
const int SplitFlapDisplay::moduleOffsets[] = {0,0,0,0,0}; //Tuning offsets (55 steps per character) +ve values move backwards on the character drum, shift flaps upwards

const int SplitFlapDisplay::SDAPin = 8;                         // SDA pin
const int SplitFlapDisplay::SCLPin = 9;                         // SCL pin

const int SplitFlapDisplay::stepsPerRotation = 2048;   //Number of stepper steps per 1 revolution of the motor, gear ratio is 1:1 to char drum
const float SplitFlapDisplay::maxVel = 15;            //Rotations Per Minute

SplitFlapDisplay::SplitFlapDisplay() { //Constructor

  // Initialize each SplitFlapModule object with the correct address
  for (uint8_t i = 0; i < numModules; i++) {
    modules[i] = SplitFlapModule(moduleAddresses[i],stepsPerRotation,moduleOffsets[i]);
  }
}

void SplitFlapDisplay::init() {
  
  Wire.begin(SDAPin,SCLPin);
  Wire.setClock(400000);

  for (uint8_t i = 0; i < numModules; i++) {
    modules[i].init();
  } 
  //Calculated Atributes
  maxStepsPerSecond = (maxVel/60) * stepsPerRotation;

}
 

void SplitFlapDisplay::runMotors() { //run motors

}

void SplitFlapDisplay::testAll() {
  char testChars[37] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  int numChars = sizeof(testChars) / sizeof(testChars[0]);
  int targetPositions[numModules];
  
  int charPos;
  for (int i = 0; i < numChars; i++) {
    // Serial.print("Target Positions: [");
    //fill array with same char

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

void SplitFlapDisplay::testRandom() {
  char testChars[37] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

  int targetPositions[numModules];
  char randChar;

  Serial.print("Target: ");
  for (int i = 0; i < numModules; i++) {
    randChar = testChars[random(0, 37)];
    targetPositions[i] = modules[i].getCharPosition(randChar);
    Serial.print(randChar);
  }
  Serial.println(" ");
  moveTo(targetPositions);

}

void SplitFlapDisplay::testCount() {

  int count = 0;
  int maxCount = pow(10,numModules);
  char targetChar;
  int targetInteger;

  int targetPositions[numModules];

  for (int i = 0; i < maxCount; i++) {
    //get each character in the count integer
    for (int j = 0; j < numModules; j++) { 
      targetInteger = (i % (int)pow(10,j+1)) / (int)pow(10,j);
      targetChar = targetInteger + '0'; //convert to char
      targetPositions[numModules-j-1] = modules[j].getCharPosition(targetChar);
    }

    moveTo(targetPositions);
    delay(250);

  } 
}

void SplitFlapDisplay::home() {
  Serial.println("Homing");
  int targetPositions[numModules];
  for (int i = 0; i < numModules; i++) { 
    targetPositions[i] = (modules[i].getPosition() - 1 + stepsPerRotation) % stepsPerRotation;
  }
  startMotors();
  moveTo(targetPositions,false);
  char homeChar = ' ';
  int charPosition;
  for (int i = 0; i < numModules; i++) { 
    targetPositions[i] = modules[i].getCharPosition(homeChar);
  }
  moveTo(targetPositions);
}

void SplitFlapDisplay::write(String inputString) {

  int inputLength = inputString.length();

  // Ensure the input string does not exceed the number of modules
  if (inputLength > numModules) {
    Serial.println("Error: Input string exceeds the number of display modules.");
    return;
  }

  int targetPositions[inputLength];
  // Iterate through the input string and process each character
  for (int i = 0; i < inputString.length(); i++) {
    char currentChar = inputString[i];
    // Serial.println(currentChar);
    targetPositions[i] = modules[i].getCharPosition(currentChar);
  }
  moveTo(targetPositions);
}

void SplitFlapDisplay::moveTo(int targetPositions[], bool releaseMotors) {

  //TODO check length of array and return if empty

  int checkIntervalUs = 20 * 1000;//How often to check each modules hall effect sensor, less than 20ms causes issues with bouncing
  int startStopDelay = 200; //time to wait to let motor realign itself to magnetic field on stop and start

  bool resetLatches[numModules]; //start with latch on to prevent case where the motion starts with the magnet over the sensor
  unsigned long lastStepTimes[numModules]; //track when each module was last stepped
  int stepDelays[numModules]; //how long to wait between each modules step

  unsigned long currentTime =  micros();
  unsigned long lastSensorCheckTime = currentTime; //track when we last read all the hall effect sensors
  bool isFinished = true; //set true to be proved false
  for (int i = 0; i < numModules; i++) { 
    targetPositions[i] = constrain(targetPositions[i], 0, stepsPerRotation-1); //Constrain to avoid errors with incorrect inputs
    resetLatches[i] = true; //set all to true, prevents case where the motion starts with the magnet over the sensor
    lastStepTimes[i] = currentTime;
    stepDelays[i] = 0;

    if (modules[i].getPosition() != targetPositions[i]) {
      isFinished = false;
    }

  }
  // startMotors(); //not sure if this helps or not, likely that it does not based on testing
  // delay(startStopDelay); //give the motor time to align to magnetic field

  while (!isFinished) {
    currentTime =  micros();
    if ((currentTime - lastSensorCheckTime) > checkIntervalUs) { //check hall effect sensor every checkIntervalMs
      //check every modules sensor
      for (int i = 0; i < numModules; i++) { 
        if (modules[i].readHallEffectSensor() == true) {
          if (!resetLatches[i]){
            Serial.print("Module: ");
            Serial.print(i);
            Serial.print(" Magnet Position: ");
            Serial.print(modules[i].getMagnetPosition());
            Serial.print(" Actual Position: ");
            Serial.print(modules[i].getPosition());
            Serial.print(" Error: ");
            Serial.println((modules[i].getMagnetPosition() - modules[i].getPosition()));
            modules[i].magnetDetected(); //update position to the modules magnet position
            resetLatches[i] = true; 
          }
        }
        else {
          resetLatches[i] = false;
        }
      } 
      lastSensorCheckTime = micros(); //recall micros because for loop may take a moment to execute
    }
    
    isFinished = true; //to be checked and set false later if positions are not correct
    for (int i = 0; i < numModules; i++) { 
      if (modules[i].getPosition() != targetPositions[i]) { //this module is not in the correct position, requires stepping
        isFinished = false;

        if ((currentTime - lastStepTimes[i]) > stepDelays[i]) {
          modules[i].step();
          stepDelays[i] = 1000000/maxStepsPerSecond; //number of microseconds to wait until the next step
          lastStepTimes[i] = currentTime;
        }
      }
    }
    // yield();
  }
  if (releaseMotors) {
    delay(startStopDelay); //allow all motors time to settle
    stopMotors();
  }

}


void SplitFlapDisplay::startMotors() { //Probably broken somewhere, not sure why, haven't looked
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
