/*
 * PoweredUpHub.h - Arduino Library for controlling PoweredUp hubs (Train, Batmobil)
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#ifndef PoweredUpHub_h
#define PoweredUpHub_h

#include "Arduino.h"
#include "BLEDevice.h"
#include "Lpf2Hub.h"

class PoweredUpHub : public Lpf2Hub
{
public:
  //Port definitions specific to PowerdUp Hubs
  enum Port
  {
    A = 0x00,
    B = 0x01
  };

  //Constructor
  PoweredUpHub();

  //Methods
  void stopMotor(Port port);
  void setMotorSpeed(Port port, int speed);
};

#endif