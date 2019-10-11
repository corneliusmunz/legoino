/*
 * BoostHub.cpp - Arduino Library for controlling Boost hubs
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "BoostHub.h"

BoostHub::BoostHub(){};

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 */
void BoostHub::setMotorSpeed(Port port, int speed)
{
    byte setMotorCommand[8] = {0x81, port, 0x11, 0x01, MapSpeed(speed), 0x64, 0x7f, 0x03}; //boost
    WriteValue(setMotorCommand, 8);
}

/**
 * @brief Set the acceleration profile 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] time Time value in ms of the acceleration from 0-100% speed/Power
 * @param [in] profileNumber Number for which the acceleration profile is stored
 */
void BoostHub::setAccelerationProfile(Port port, int16_t time, int8_t profileNumber)
{
    byte *timeBytes = Int16ToByteArray(time);
    byte setMotorCommand[7] = {0x81, port, 0x10, 0x05, timeBytes[0], timeBytes[1], profileNumber};
    WriteValue(setMotorCommand, 7);
}

/**
 * @brief Set the deceleration profile 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] time Time value in ms of the deceleration from 100-0% speed/Power
 * @param [in] profileNumber Number for which the deceleration profile is stored
 */
void BoostHub::setDecelerationProfile(Port port, int16_t time, int8_t profileNumber)
{
    byte *timeBytes = Int16ToByteArray(time);
    byte setMotorCommand[7] = {0x81, port, 0x10, 0x06, timeBytes[0], timeBytes[1], profileNumber};
    WriteValue(setMotorCommand, 7);
}

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 * @param [in] time Time in miliseconds for running the motor on the desired speed
 */
void BoostHub::setMotorSpeedForTime(Port port, int speed, int16_t time = 0)
{
    //max power 100 (0x64)
    //End state Brake (127)
    //Use acc and dec profile (0x03 last two bits set)
    byte *timeBytes = Int16ToByteArray(time);
    byte setMotorCommand[10] = {0x81, port, 0x11, 0x09, timeBytes[0], timeBytes[1], MapSpeed(speed), 0x64, 0x7F, 0x03}; //boost with time
    WriteValue(setMotorCommand, 10);
}

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 * @param [in] time Time in miliseconds for running the motor on the desired speed
 */
void BoostHub::setMotorSpeedForDegrees(Port port, int speed, int32_t degrees)
{
    byte *degreeBytes = Int32ToByteArray(degrees);
    //max power 100 (0x64)
    //End state Brake (127)
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[12] = {0x81, port, 0x11, 0x0B, degreeBytes[0], degreeBytes[1], degreeBytes[2], degreeBytes[3], MapSpeed(speed), 0x64, 0x7F, 0x03}; //boost with time
    WriteValue(setMotorCommand, 12);
}

void BoostHub::setMotorSpeedsForDegrees(int speedLeft, int speedRight, int32_t degrees)
{
    byte *degreeBytes = Int32ToByteArray(degrees);
    Port port = AB;
    //both ports A and B 
    //max power 100 (0x64)
    //End state Brake (127)
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[13] = {0x81, port, 0x11, 0x0C, degreeBytes[0], degreeBytes[1], degreeBytes[2], degreeBytes[3], MapSpeed(speedLeft), MapSpeed(speedRight), 0x64, 0x7F, 0x03}; //boost with time
    WriteValue(setMotorCommand, 13);
}

void BoostHub::requestSensorValue()
{
    byte requestPortValue[3] = {0x21, 0x01, 0x00};
    WriteValue(requestPortValue, 3);
}

void BoostHub::setInputFormatSingle(){
    //byte inputFormatValue[8] = {0x41, 0x01, 0x08, 0x01, 0x00, 0x00, 0x00, 0x01}; //color and distance on port C (1)
    byte inputFormatValue[8] = {0x41, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x01}; //boost tacho motor on port C (1)
    WriteValue(inputFormatValue, 8);
}

/**
 * @brief Stop the motor on a defined port. If no port is set, all motors (AB) will be stopped
 * @param [in] port Port of the Hub on which the motor will be stopped (A, B, AB, C, D)
 */
void BoostHub::stopMotor(Port port = AB)
{
    setMotorSpeed(port, 0);
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
    setMotorSpeedForDegrees(port, 50, steps * 360 * 2);
}

/**
 * @brief Move back (Port AB) with the default speed and stop after the number of steps
 * @param [in] steps Number of steps (Boost grid)
 */
void BoostHub::moveBack(int steps)
{
    Port port = AB;
    setMotorSpeedForDegrees(port, -50, steps * 360 * 2);
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
