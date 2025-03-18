#ifndef CONFIG_H
#define CONFIG_H

// Settings for the SplitFlapDisplay
// Customize these settings to match your hardware prior to compiling and uploading to your device
//
// If you are contributing to the project, please do not commit any changes to this file
// Instead, copy this file to config.h in the same directory and make your changes there
// This will prevent your settings from being committed to the repository

#define NUM_MODULES 8;
#define MODULE_ADDRESSES {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};  // Module I2C Addresses, In Order

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

#define MAGNET_POSITION 730; //position of character drum when the magnet is detected, this sets position 0 to be the blank flap
#define MODULE_OFFSETS {0, 0, 0, 0, 0, 0, 0, 0}; //Tuning offsets (55 steps per character) +ve values move backwards on the character drum, shift flaps upwards
#define DISPLAY_OFFSET 0; //Offset for all modules, for final adjustment

#define SDA_PIN 8; // SDA pin
#define SCL_PIN 9; // SCL pin

#define STEPS_PER_ROTATION 2048; //Number of stepper steps per 1 revolution of the motor, gear ratio is 1:1 to char drum
#define MAX_VEL 15.0f; //Max Rotations Per Minute

#define NTP_SERVER "pool.ntp.org"; // NTP Server
#define TZ_INFO "GMT0BST,M3.5.0/1,M10.5.0";  // UK: GMT in winter, BST (UTC+1) in summer

#endif
