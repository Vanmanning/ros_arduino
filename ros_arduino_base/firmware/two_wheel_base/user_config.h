#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

// Select your motor driver here
#define PololuMC33926
//#define DFRobotL298PShield
// Define your encoder pins here.
// Try to use pins that have interrupts
#define ENCODER1_A 2  // Interrupt on Teensy 3.0
#define ENCODER1_B 5  // Interrupt on Teensy 3.0

#define ENCODER2_A 15  // Interrupt on Teensy 3.0
#define ENCODER2_B 16  // Interrupt on Teensy 3.0

#define ENCODER3_A 17  // Interrupt on Teensy 3.0
#define ENCODER3_B 18  // Interrupt on Teensy 3.0

#define LED 3 // LED indicator pin

#endif  // _USER_CONFIG_H_
