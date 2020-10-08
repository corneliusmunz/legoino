/*
 * DuploTrainHub.cpp - Arduino Library for controlling Duplo Train hubs (10874, 10875)
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "DuploTrainHub.h"

DuploTrainHub::DuploTrainHub(){};

void DuploTrainHub::playSound(byte sound)
{
    byte setSoundMode[8] = {0x41, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(setSoundMode, 8);
    byte playSound[6] = {0x81, 0x01, 0x11, 0x51, 0x01, sound};
    WriteValue(playSound, 6);
}

void DuploTrainHub::playTone(byte number)
{
    byte setToneMode[8] = {0x41, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(setToneMode, 8);
    byte playTone[6] = {0x81, 0x01, 0x11, 0x51, 0x02, number};
    WriteValue(playTone, 6);
}

