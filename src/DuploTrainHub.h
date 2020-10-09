/*
 * DuploTrainHub.h - Arduino Library for controlling Duplo train hubs (10874, 10875)
 *
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 *
*/

#ifndef DuploTrainHub_h
#define DuploTrainHub_h

#include "Lpf2Hub.h"

class DuploTrainHub : public Lpf2Hub
{
public:
  //Port definitions
  enum Port
  {
    MOTOR = 0x00,
    LED = 0x11,
    SPEAKER = 0x01,
    COLOR = 0x12,
    SPEEDOMETER = 0x13,
    VOLTAGE = 0x14
  };

  //Constructor
  DuploTrainHub();

  //Sound methods
  void playSound(byte sound);
  void playTone(byte number);


};

#endif