/*
 * TechnicHub.h - Arduino Library for controlling Technic hubs
 *
 * (c) Copyright 2019 - Cornelius Munz
 * Technic hub - Jeremy Williams
 * Released under MIT License
 *
*/

#ifndef TechnicHub_h
#define TechnicHub_h

#include "Arduino.h"
#include "BLEDevice.h"
#include "Lpf2Hub.h"

class TechnicHub : public Lpf2Hub
{
public:
    //Port definitions specific to Technic Hubs
  enum Port
  {
    A = 0x00,
    B = 0x01,
    C = 0x02,
    D = 0x03,
    CURRENT = 0x3B,
    VOLTAGE = 0x3C
  };

  //Constructor
  TechnicHub();

  //Basic Hub methods
  void requestSensorValue();
  void setInputFormatSingle();

  //Basic Motor methods
  void setAccelerationProfile(Port port, int16_t time, int8_t profileNumber);
  void setDecelerationProfile(Port port, int16_t time, int8_t profileNumber);
  void stopMotor(Port port);
  void setMotorSpeed(Port port, int speed);
  void setMotorSpeedForTime(Port port, int speed, int16_t time);
  void setMotorSpeedForDegrees(Port port, int speed, int32_t degrees);
  void setMotorSpeedsForDegrees(int speedLeft, int speedRight, int32_t degrees);

  //Basic Move/Rotate methods
  void moveForward(int steps);
  void moveBack(int steps);
  void rotate(int degrees);
  void rotateLeft(int degrees);
  void rotateRight(int degrees);
  void moveArc(int degrees);
  void moveArcLeft(int degrees);
  void moveArcRight(int degrees);
};

#endif
