// Blocking.pde
// -*- mode: C++ -*-
//
// Shows how to use the blocking call runToNewPosition
// Which sets a new target position and then waits until the stepper has 
// achieved it.
//
// Copyright (C) 2009 Mike McCauley
// $Id: Blocking.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

// Define a stepper and the pins it will use
//AccelStepper stepper(AccelStepper::HALF4WIRE, 5, 6, 7, 8);
//AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper stepper(1,3,2);

void setup()
{   
    //pinMode(11,OUTPUT);  //standby
    //pinMode(9,OUTPUT);    //PWM
    //pinMode(10,OUTPUT);
    //digitalWrite(11,HIGH);
    //analogWrite(9, 200);
    //analogWrite(10, 200);
    pinMode(4,OUTPUT);
    digitalWrite(4,HIGH);

    
    stepper.setMinPulseWidth(100);
    
    stepper.setMaxSpeed(500.0);
    stepper.setAcceleration(100.0);
}

void loop()
{    
    //stepper.runToNewPosition(0);
    stepper.runToNewPosition(100000);
    //stepper.run();
}
