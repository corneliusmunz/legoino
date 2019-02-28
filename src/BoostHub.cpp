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
    //byte setMotorCommand[10] = {0xA, 0x00, 0x81, port, 0x11, 0x60, 0x00, MapSpeed(speed), 0x00, 0x00};
    //_pRemoteCharacteristic->writeValue(setMotorCommand, sizeof(setMotorCommand), false);
    //byte setMotorCommand[8] = {0x81, port, 0x11, 0x60, 0x00, MapSpeed(speed), 0x00, 0x00}; //train
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
    //byte setMotorCommand[10] = {0xA, 0x00, 0x81, port, 0x11, 0x60, 0x00, MapSpeed(speed), 0x00, 0x00};
    //_pRemoteCharacteristic->writeValue(setMotorCommand, sizeof(setMotorCommand), false);
    //byte setMotorCommand[8] = {0x81, port, 0x11, 0x60, 0x00, MapSpeed(speed), 0x00, 0x00}; //train
    //byte setMotorCommand[8] = {0x81, port, 0x11, 0x01, MapSpeed(speed), 0x64, 0x7f, 0x03}; //boost
    byte *timeBytes = Int16ToByteArray(time);
    //max power 100 (0x64)
    //End state Brake (127)
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[10] = {0x81, port, 0x11, 0x09, timeBytes[0], timeBytes[1], MapSpeed(speed), 0x64, 0x7F, 0x03}; //boost with time
    WriteValue(setMotorCommand, 10);
}

void BoostHub::setMotorSpeedForDegrees(Port port, int speed, int32_t degrees)
{
    //byte setMotorCommand[10] = {0xA, 0x00, 0x81, port, 0x11, 0x60, 0x00, MapSpeed(speed), 0x00, 0x00};
    //_pRemoteCharacteristic->writeValue(setMotorCommand, sizeof(setMotorCommand), false);
    //byte setMotorCommand[8] = {0x81, port, 0x11, 0x60, 0x00, MapSpeed(speed), 0x00, 0x00}; //train
    //byte setMotorCommand[8] = {0x81, port, 0x11, 0x01, MapSpeed(speed), 0x64, 0x7f, 0x03}; //boost
    byte *degreeBytes = Int32ToByteArray(degrees);
    //max power 100 (0x64)
    //End state Brake (127)
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[12] = {0x81, port, 0x11, 0x0B, degreeBytes[0], degreeBytes[1], degreeBytes[2], degreeBytes[3], MapSpeed(speed), 0x64, 0x7F, 0x03}; //boost with time
    WriteValue(setMotorCommand, 12);
}

/**
 * @brief Stop the motor on a defined port. If no port is set, all motors (AB) will be stopped
 * @param [in] port Port of the Hub on which the motor will be stopped (A, B, AB)
 */
void BoostHub::stopMotor(Port port)
{
    setMotorSpeed(port, 0);
}
