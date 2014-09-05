#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_


// Select your motor driver here
#define PololuMC33926
//#define DFRobotL298PShield
#include "motor_driver_config.h"

// Define your encoder pins here.
// Try to use pins that have interrupts
// Left side encoders pins
#define LEFT_ENCODER_A 14  // Interrupt on Teensy 3.0
#define LEFT_ENCODER_B 15  // Interrupt on Teensy 3.0
// Right side encoders pins
#define RIGHT_ENCODER_A 6  // Interrupt on Teensy 3.0
#define RIGHT_ENCODER_B 7  // Interrupt on Teensy 3.0

// Are you using an IMU?
#define IMU

#if defined(IMU)
 #include "imu_config.h"
#endif

#endif  // _USER_CONFIG_H_
