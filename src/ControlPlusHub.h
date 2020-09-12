/*
 * ControlPlusHub.h - Arduino Library for controlling Technic hubs
 *
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 *
*/

#ifndef ControlPlusHub_h
#define ControlPlusHub_h

#include "Arduino.h"
#include "NimBLEDevice.h"
#include "Lpf2Hub.h"

class ControlPlusHub : public Lpf2Hub
{
public:
  //Port definitions specific to ControlPlus Hubs
  enum Port
  {
    A = 0x00,
    B = 0x01,
    C = 0x02,
    D = 0x03,
    HUB_LED = 0x32,
    CURRENT = 0x3B,
    VOLTAGE = 0x3C,
    ACCELEROMETER = 0x61,
    GYRO = 0x62,
    TILT = 0x63
  };

  //Constructor
  ControlPlusHub();

  //Basic Motor methods
  void setAccelerationProfile(Port port, int16_t time, int8_t profileNumber);
  void setDecelerationProfile(Port port, int16_t time, int8_t profileNumber);
  void stopMotor(Port port);
  void setMotorSpeed(Port port, int speed);
  void setMotorSpeedForTime(Port port, int speed, int16_t time);
  void setMotorSpeedForDegrees(Port port, int speed, int32_t degrees);
};

#endif