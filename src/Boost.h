/*
 * Boost.h - Arduino Library for controlling LEGO® Boost Model (17101)
 * It has included some higher level abstratctions for moving one step forward/back
 * rotate the model or move with an arc. 
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#ifndef Boost_h
#define Boost_h

#include "Lpf2Hub.h"

class Boost : public Lpf2Hub
{
public:
  //Constructor
  Boost();

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