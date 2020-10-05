/*
 * BoostHub.cpp - Arduino Library for controlling Boost hubs
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "BoostHub.h"

BoostHub::BoostHub(){};


void BoostHub::setMotorSpeedsForDegrees(int speedLeft, int speedRight, int32_t degrees)
{
    byte *degreeBytes = LegoinoCommon::Int32ToByteArray(degrees);
    Port port = AB;
    //both ports A and B
    //max power 100 (0x64)
    //End state Brake (127)
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[13] = {0x81, port, 0x11, 0x0C, degreeBytes[0], degreeBytes[1], degreeBytes[2], degreeBytes[3], LegoinoCommon::MapSpeed(speedLeft), LegoinoCommon::MapSpeed(speedRight), 0x64, 0x7F, 0x03}; //boost with time
    WriteValue(setMotorCommand, 13);
}


/**
 * @brief Move forward (Port AB) with the default speed and stop after the number of steps
 * @param [in] steps Number of steps (Boost grid)
 */
void BoostHub::moveForward(int steps)
{
    Port port = AB;
    Port portA = A;
    Port portB = B;
    setDecelerationProfile(portA, 1000, 0);
    setDecelerationProfile(portB, 1000, 0);
    setTachoMotorSpeedForDegrees(port, 50, steps * 360 * 2);
}

/**
 * @brief Move back (Port AB) with the default speed and stop after the number of steps
 * @param [in] steps Number of steps (Boost grid)
 */
void BoostHub::moveBack(int steps)
{
    Port port = AB;
    setTachoMotorSpeedForDegrees(port, -50, steps * 360 * 2);
}

/**
 * @brief rotate (Port AB) with the default speed and stop after the degrees
 * @param [in] degrees (negative: left, positive: right)
 */
void BoostHub::rotate(int degrees)
{
    if (degrees > 0)
    {
        // right
        setMotorSpeedsForDegrees(-50, 50, degrees * 4.5);
    }
    else
    {
        // left
        setMotorSpeedsForDegrees(50, -50, degrees * 4.5);
    }
}

/**
 * @brief rotate left (Port AB) with the default speed and stop after degrees (default 90)
 * @param [in] degrees (default 90)
 */
void BoostHub::rotateLeft(int degrees = 90)
{
    rotate(-degrees);
}

/**
 * @brief rotate right (Port AB) with the default speed and stop after degrees (default 90)
 * @param [in] degrees (default 90)
 */
void BoostHub::rotateRight(int degrees = 90)
{
    rotate(degrees);
}

/**
 * @brief move an arc (Port AB) with the default speed and stop after degrees
 * @param [in] degrees (negative: left, positive: right)
 */
void BoostHub::moveArc(int degrees)
{
    if (degrees > 0)
    {
        // right
        setMotorSpeedsForDegrees(60, 20, degrees * 12);
    }
    else
    {
        // left
        setMotorSpeedsForDegrees(20, 60, degrees * 12);
    }
}

/**
 * @brief move an arc left (Port AB) with the default speed and stop after degrees (default 90)
 * @param [in] degrees (default 90)
 */
void BoostHub::moveArcLeft(int degrees = 90)
{
    moveArc(-degrees);
}

void BoostHub::moveArcRight(int degrees = 90)
{
    moveArc(degrees);
}
