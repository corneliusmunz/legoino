/*
 * MoveHub.h - Arduino Library for controlling LEGO® Move Hub (88006)
 *
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 *
*/

#ifndef MoveHub_h
#define MoveHub_h

#include "Lpf2Hub.h"

class MoveHub : public Lpf2Hub
{
public:
  //Port definitions specific to Boost Hubs
  enum Port
  {
    A = 0x00,
    B = 0x01,
    AB = 0x10,
    C = 0x02,
    D = 0x03,
    TILT = 0x3A,
    CURRENT = 0x3B,
    VOLTAGE = 0x3C
  };

  //Constructor
  MoveHub();

  //Basic Motor methods
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