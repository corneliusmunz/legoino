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