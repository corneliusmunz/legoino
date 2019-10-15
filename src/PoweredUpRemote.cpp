/*
 * PoweredUpRemote.cpp - Arduino Library for controlling PoweredUp remote controls (train)
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "PoweredUpRemote.h"

PoweredUpRemote::PoweredUpRemote(){};

/**
 * @brief Set the color of the HUB LED with predefined colors
 * @param [in] color one of the available hub colors
 */
void PoweredUpRemote::setLedColor(Color color) 
{
    byte setColorMode[8] = {0x41, 0x34, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(setColorMode, 8);
    byte setColor[6] = {0x81, 0x34, 0x11, 0x51, 0x00, color};
    WriteValue(setColor, 6);
}

/**
 * @brief Set the color of the HUB LED with RGB values 
 * @param [in] red 0..255 
 * @param [in] green 0..255 
 * @param [in] blue 0..255 
 */
void PoweredUpRemote::setLedRGBColor(char red, char green, char blue)
{
    byte setRGBMode[8] = {0x41, 0x34, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(setRGBMode, 8);
    byte setRGBColor[8] = {0x81, 0x34, 0x11, 0x51, 0x01, red, green, blue};
    WriteValue(setRGBColor, 8);
}
