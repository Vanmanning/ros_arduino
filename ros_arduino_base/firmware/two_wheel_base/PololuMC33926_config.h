#ifndef _PololuMC33926_CONFIG_H_
#define _PololuMC33926_CONFIG_H_

#include "motor_driver_config.h"
#include "PololuMC33926.h"

// Left and Right motor driver objects
MC33926 left_motor(22,21,23,19);
MC33926 right_motor(14,13,20,19);

void setupMotors()
{
  // Initalize Motors
  left_motor.init();
  right_motor.init();
  
  pinMode(19, OUTPUT);
  digitalWrite(19, HIGH);
}
void commandLeftMotor(int cmd)
{
  left_motor.set_pwm(cmd);
}
void commandRightMotor(int cmd)
{
  right_motor.set_pwm(cmd);
}

#endif  // _PololuMC33926_CONFIG_H_

