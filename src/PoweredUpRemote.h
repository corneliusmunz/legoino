/*
 * PoweredUpRemote.h - Arduino Library for controlling PoweredUp remote controls (train)
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#ifndef PoweredUpRemote_h
#define PoweredUpRemote_h

#include "Arduino.h"
#include "BLEDevice.h"
#include "Lpf2Hub.h"

class PoweredUpRemote : public Lpf2Hub
{
public:
  //Port definitions specific to PowerdUp Remote
  enum Port
  {
    LEFT = 0x00,
    RIGHT = 0x01
  };

  //Constructor
  PoweredUpRemote();

  //Methods
  void setLedColor(Color color);
  void setLedRGBColor(char red, char green, char blue);

};

#endif