/*
 * LegoinoCommon.h - Arduino Library for converting values with different types
 *
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 *
*/

#if defined(ESP32)

#ifndef LegoinoCommon_h
#define LegoinoCommon_h

#include "Arduino.h"
#include "Lpf2HubConst.h"

class LegoinoCommon
{
public:
  static byte MapSpeed(int speed);
  static byte *Int16ToByteArray(int16_t x);
  static byte *Int32ToByteArray(int32_t x);
  static unsigned char ReadUInt8(uint8_t *data, int offset);
  static signed char ReadInt8(uint8_t *data, int offset);
  static unsigned short ReadUInt16LE(uint8_t *data, int offset);
  static signed short ReadInt16LE(uint8_t *data, int offset);
  static unsigned int ReadUInt32LE(uint8_t *data, int offset);
  static signed int ReadInt32LE(uint8_t *data, int offset);
  static std::string ColorStringFromColor(Color color);
  static std::string ColorStringFromColor(int color);
};

#endif // LegoinoCommon_h

#endif // ESP32
