#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const int NUM_MODULES = 8;
const uint8_t MODULE_ADDRESSES[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};  // Module I2C Addresses, In Order

//Solder the contacts on PCF8575 for these addresses
// ADDR  | A0 | A1 | A2
// ----------------------
// 0x20  | L  | L  | L
// 0x21  | H  | L  | L
// 0x22  | L  | H  | L
// 0x23  | H  | H  | L
// 0x24  | L  | L  | H
// 0x25  | H  | L  | H
// 0x26  | L  | H  | H
// 0x27  | H  | H  | H

const int MAGNET_POSITION = 730; //position of character drum when the magnet is detected, this sets position 0 to be the blank flap
const int MODULE_OFFSETS[] = {0, 0, 0, 0, 0, 0, 0, 0}; //Tuning offsets (55 steps per character) +ve values move backwards on the character drum, shift flaps upwards

const int SDA_PIN = 8; // SDA pin
const int SCL_PIN = 9; // SCL pin

const int STEPS_PER_ROTATION = 2048; //Number of stepper steps per 1 revolution of the motor, gear ratio is 1:1 to char drum
const float MAX_VEL = 15.0f; //Max Rotations Per Minute

#endif
