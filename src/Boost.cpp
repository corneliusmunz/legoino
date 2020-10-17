/*
 * Boost.cpp - Arduino Library for controlling LEGO® Boost Model (17101)
 * It has included some higher level abstratctions for moving one step forward/back
 * rotate the model or move with an arc. 
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "Boost.h"

Boost::Boost(){};

/**
 * @brief Move forward (Port AB) with the default speed and stop after the number of steps
 * @param [in] steps Number of steps (Boost grid)
 */
void Boost::moveForward(int steps)
{
    byte port = (byte)MoveHubPort::AB;
    byte portA = (byte)MoveHubPort::A;
    byte portB = (byte)MoveHubPort::B;

    setDecelerationProfile(portA, 1000);
    setDecelerationProfile(portB, 1000);
    setTachoMotorSpeedForDegrees(port, 50, steps * 360 * 2);
}

/**
 * @brief Move back (Port AB) with the default speed and stop after the number of steps
 * @param [in] steps Number of steps (Boost grid)
 */
void Boost::moveBack(int steps)
{
    byte port = (byte)MoveHubPort::AB;
    setTachoMotorSpeedForDegrees(port, -50, steps * 360 * 2);
}

/**
 * @brief rotate (Port AB) with the default speed and stop after the degrees
 * @param [in] degrees (negative: left, positive: right)
 */
void Boost::rotate(int degrees)
{
    if (degrees > 0)
    {
        // right
        setTachoMotorSpeedsForDegrees(-50, 50, degrees * 4.5);
    }
    else
    {
        // left
        setTachoMotorSpeedsForDegrees(50, -50, degrees * 4.5);
    }
}

/**
 * @brief rotate left (Port AB) with the default speed and stop after degrees (default 90)
 * @param [in] degrees (default 90)
 */
void Boost::rotateLeft(int degrees = 90)
{
    rotate(-degrees);
}

/**
 * @brief rotate right (Port AB) with the default speed and stop after degrees (default 90)
 * @param [in] degrees (default 90)
 */
void Boost::rotateRight(int degrees = 90)
{
    rotate(degrees);
}

/**
 * @brief move an arc (Port AB) with the default speed and stop after degrees
 * @param [in] degrees (negative: left, positive: right)
 */
void Boost::moveArc(int degrees)
{
    if (degrees > 0)
    {
        // right
        setTachoMotorSpeedsForDegrees(60, 20, degrees * 12);
    }
    else
    {
        // left
        setTachoMotorSpeedsForDegrees(20, 60, degrees * 12);
    }
}

/**
 * @brief move an arc left (Port AB) with the default speed and stop after degrees (default 90)
 * @param [in] degrees (default 90)
 */
void Boost::moveArcLeft(int degrees = 90)
{
    moveArc(-degrees);
}

void Boost::moveArcRight(int degrees = 90)
{
    moveArc(degrees);
}
