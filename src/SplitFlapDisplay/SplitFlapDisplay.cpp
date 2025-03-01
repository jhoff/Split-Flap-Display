#include "config.h"
#include "SplitFlapDisplay.h"
#include "SplitFlapModule.h"

// Edit these values in config.h
const int SplitFlapDisplay::numModules = NUM_MODULES;
const uint8_t* SplitFlapDisplay::moduleAddresses = MODULE_ADDRESSES;
const int SplitFlapDisplay::magnetPosition = MAGNET_POSITION;
const int* SplitFlapDisplay::moduleOffsets = MODULE_OFFSETS;
const int SplitFlapDisplay::SDAPin = SDA_PIN;
const int SplitFlapDisplay::SCLPin = SCL_PIN;
const int SplitFlapDisplay::stepsPerRotation = STEPS_PER_ROTATION;
const float SplitFlapDisplay::maxVel = MAX_VEL;

SplitFlapDisplay::SplitFlapDisplay() { //Constructor

  // Initialize each SplitFlapModule object with the correct address
  for (uint8_t i = 0; i < numModules; i++) {
    modules[i] = SplitFlapModule(moduleAddresses[i],stepsPerRotation,moduleOffsets[i],magnetPosition);
  }
}

void SplitFlapDisplay::init() {

  Wire.begin(SDAPin,SCLPin);
  Wire.setClock(400000);

  for (uint8_t i = 0; i < numModules; i++) {
    modules[i].init();
  }

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

void SplitFlapDisplay::testRandom(float speed) {
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
  moveTo(targetPositions,speed);

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

void SplitFlapDisplay::home(float speed) {
  Serial.println("Homing");
  int targetPositions[numModules];
  for (int i = 0; i < numModules; i++) {
    targetPositions[i] = (modules[i].getPosition() - 1 + stepsPerRotation) % stepsPerRotation;
  }
  startMotors();
  moveTo(targetPositions,speed,false);
  char homeChar = ' ';
  int charPosition;
  for (int i = 0; i < numModules; i++) {
    targetPositions[i] = modules[i].getCharPosition(homeChar);
  }
  moveTo(targetPositions,speed);
}

void SplitFlapDisplay::homeToString(String homeString,float speed,bool centering) {
  Serial.println("Homing");
  int targetPositions[numModules];
  for (int i = 0; i < numModules; i++) {
    targetPositions[i] = (modules[i].getPosition() - 1 + stepsPerRotation) % stepsPerRotation;
  }
  startMotors();
  moveTo(targetPositions,speed,false);
  writeString(homeString,speed,centering);
}

void SplitFlapDisplay::homeToChar(char homeChar,float speed) {
  Serial.println("Homing");
  int targetPositions[numModules];
  for (int i = 0; i < numModules; i++) {
    targetPositions[i] = (modules[i].getPosition() - 1 + stepsPerRotation) % stepsPerRotation;
  }
  startMotors();
  moveTo(targetPositions,speed,false);

  for (int i = 0; i < numModules; i++) {
    targetPositions[i] = modules[i].getCharPosition(homeChar);
  }
  moveTo(targetPositions,true,speed);
}

void SplitFlapDisplay::writeChar(char inputChar,float speed) {

  int targetPositions[numModules];
  // Iterate through the input string and process each character
  for (int i = 0; i < numModules; i++) {
    targetPositions[i] = modules[i].getCharPosition(inputChar);
  }
  moveTo(targetPositions,speed);

}

void SplitFlapDisplay::writeString(String inputString,float speed,bool centering) {

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
  }
  else{ //pad blanks to end, if no centering
    while (displayString.length() < numModules) { // Pad with spaces
      displayString += " ";  // Padding with space
    }
  }

  int targetPositions[numModules];
  // Iterate through the input string and process each character
  for (int i = 0; i < displayString.length(); i++) {
    char currentChar = displayString[i];
    // Serial.println(currentChar);
    targetPositions[i] = modules[i].getCharPosition(currentChar);
  }
  moveTo(targetPositions,speed);

}

void SplitFlapDisplay::moveTo(int targetPositions[],float speed, bool releaseMotors) {

  //TODO check length of array and return if empty

  speed = constrain(speed,2,maxVel);
  float stepsPerSecond = (speed/60) * stepsPerRotation;
  float timePerStep = 1000000/stepsPerSecond;

  unsigned long currentTime =  micros();

  int checkIntervalUs = 20 * 1000;//How often to check each modules hall effect sensor, less than 20ms causes issues with bouncing
  int startStopDelay = 200; //time to wait to let motor realign itself to magnetic field on stop and start

  bool resetLatches[numModules]  = {};  // Initialize to false //start with latch on to prevent case where the motion starts with the magnet over the sensor
  bool needsStepping[numModules] = {};  // Initialize to false; //modules that still require moving
  unsigned long lastStepTimes[numModules] = {};  // Initialize to false; //track when each module was last stepped
  unsigned long lastSensorCheckTime = currentTime; //track when we last read all the hall effect sensors

  for (int i = 0; i < numModules; i++) {
    targetPositions[i] = constrain(targetPositions[i], 0, stepsPerRotation-1); //Constrain to avoid errors with incorrect inputs
    resetLatches[i] = true;
    lastStepTimes[i] = currentTime;
    if (modules[i].getPosition() != targetPositions[i]) {
      needsStepping[i] = true;
    }
    else{
      needsStepping[i] = false;
    }
  }

  startMotors(); //not sure if this helps or not, likely that it does not based on testing
  delay(startStopDelay); //give the motor time to align to magnetic field

  bool isFinished = checkAllFalse(needsStepping,numModules);
  while (!isFinished) {

    currentTime =  micros();
    for (int i = 0; i < numModules; i++) {
      if (((currentTime - lastStepTimes[i]) > timePerStep) && needsStepping[i]) {
        modules[i].step();
        lastStepTimes[i] = micros();
        if (modules[i].getPosition() == targetPositions[i]) { //this module is not in the correct position, requires stepping
          needsStepping[i] = false;
        }
      }
    }

    if ((currentTime - lastSensorCheckTime) > checkIntervalUs) { //check hall effect sensor every checkIntervalMs
      //check every modules sensor
      for (int i = 0; i < numModules; i++) {
        if (needsStepping[i] && (modules[i].readHallEffectSensor() == true)) { //only check sensors where the module is still moving
          if (!resetLatches[i]){
            //UNCOMMENTING THIS WILL PROBBALY MAKE THE MOTORS INACCURATE, DUE TO TIME TAKEN TO PRINT
            // Serial.print("Module: ");
            // Serial.print(i);
            // Serial.print(" Magnet Position: ");
            // Serial.print(modules[i].getMagnetPosition());
            // Serial.print(" Actual Position: ");
            // Serial.print(modules[i].getPosition());
            // Serial.print(" Error: ");
            // Serial.println((modules[i].getMagnetPosition() - modules[i].getPosition()));
            modules[i].magnetDetected(); //update position to the modules magnet position
            resetLatches[i] = true;
          }
        }
        else if (resetLatches[i] == true) {
          resetLatches[i] = false;
        }
      }
      isFinished = checkAllFalse(needsStepping,numModules);
      lastSensorCheckTime = currentTime; //recall micros because for loop may take a moment to execute
    }

  }
  if (releaseMotors) {
    delay(startStopDelay); //allow all motors time to settle
    stopMotors();
  }

}

bool SplitFlapDisplay::checkAllFalse(bool array[], int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == true) {
            return false;  // As soon as a true value is found, return false
        }
    }
    return true;  // All values were false
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
