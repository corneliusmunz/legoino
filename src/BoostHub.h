/*
 * BoostHub.h - Arduino Library for controlling Boost hubs
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#ifndef BoostHub_h
#define BoostHub_h

#include "Arduino.h"
#include "BLEDevice.h"
#include "Lpf2Hub.h"


typedef enum Port {
  A = 0x37,
  B = 0x38,
  AB = 0x39,
  C = 0x01,
  D = 0x02,
  TILT = 0x3A
};


class BoostHub : public Lpf2Hub
{
  public:
    BoostHub();    
    void setAccelerationProfile(Port port, int16_t time, int8_t profileNumber);
    void setDecelerationProfile(Port port, int16_t time, int8_t profileNumber);

    void stopMotor(Port port);
    void setMotorSpeed(Port port, int speed);
    void setMotorSpeeds(int speedA, int speedB);
    void setMotorSpeedForTime(Port port, int speed, int16_t time);
    void setMotorSpeedForDegrees(Port port, int speed, int32_t degrees);

};

#endif