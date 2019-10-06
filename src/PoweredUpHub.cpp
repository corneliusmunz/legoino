/*
 * PoweredUpHub.cpp - Arduino Library for controlling PoweredUp hubs (Train, Batmobil)
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "PoweredUpHub.h"

PoweredUpHub::PoweredUpHub(){};

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 */
void PoweredUpHub::setMotorSpeed(Port port, int speed = 0)
{
    byte setMotorCommand[8] = {0x81, port, 0x11, 0x51, 0x00, MapSpeed(speed)}; //train, batmobil
    WriteValue(setMotorCommand, 6);
}

/**
 * @brief Stop the motor on a defined port. If no port is set, all motors (AB) will be stopped
 * @param [in] port Port of the Hub on which the motor will be stopped (A, B, AB)
 */
void PoweredUpHub::stopMotor(Port port = AB)
{
    setMotorSpeed(port, 0);
}
