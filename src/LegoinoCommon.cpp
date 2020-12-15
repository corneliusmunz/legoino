/*
 * LegoinoCommon.cpp - Arduino Library for converting values with different types
 *
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 *
*/

#if defined(ESP32)

#include "LegoinoCommon.h"

/**
 * @brief Map speed from -100..100 to the 8bit internal value
 * @param [in] speed -100..100
 */
byte LegoinoCommon::MapSpeed(int speed)
{
    byte rawSpeed;
    if (speed == 0)
    {
        rawSpeed = 127; // stop motor
    }
    else if (speed > 0)
    {
        rawSpeed = map(speed, 0, 100, 0, 126);
    }
    else
    {
        rawSpeed = map(-speed, 0, 100, 255, 128);
    }
    return rawSpeed;
}

/**
 * @brief return string value of color enum
 * @param [in] Color enum
 */
std::string LegoinoCommon::ColorStringFromColor(Color color)
{
    return ColorStringFromColor((int)color);
}

/**
 * @brief return string value of color enum
 * @param [in] Color int value
 */
std::string LegoinoCommon::ColorStringFromColor(int color)
{
    if (color > Color::NUM_COLORS) {
        return std::string(COLOR_STRING[Color::NUM_COLORS]);
    } 
    else 
    {
        return std::string(COLOR_STRING[color]);
    }
}

byte *LegoinoCommon::Int16ToByteArray(int16_t x)
{
    static byte y[2];
    y[0] = (byte)(x & 0xff);
    y[1] = (byte)((x >> 8) & 0xff);
    return y;
}

byte *LegoinoCommon::Int32ToByteArray(int32_t x)
{
    static byte y[4];
    y[0] = (byte)(x & 0xff);
    y[1] = (byte)((x >> 8) & 0xff);
    y[2] = (byte)((x >> 16) & 0xff);
    y[3] = (byte)((x >> 24) & 0xff);
    return y;
}

uint8_t LegoinoCommon::ReadUInt8(uint8_t *data, int offset = 0)
{
    uint8_t value = data[0 + offset];
    return value;
}

int8_t LegoinoCommon::ReadInt8(uint8_t *data, int offset = 0)
{
    int8_t value = (int8_t)data[0 + offset];
    return value;
}

uint16_t LegoinoCommon::ReadUInt16LE(uint8_t *data, int offset = 0)
{
    uint16_t value = data[0 + offset] | (uint16_t)(data[1 + offset] << 8);
    return value;
}

int16_t LegoinoCommon::ReadInt16LE(uint8_t *data, int offset = 0)
{
    int16_t value = data[0 + offset] | (int16_t)(data[1 + offset] << 8);
    return value;
}

uint32_t LegoinoCommon::ReadUInt32LE(uint8_t *data, int offset = 0)
{
    uint32_t value = data[0 + offset] | (uint32_t)(data[1 + offset] << 8) | (uint32_t)(data[2 + offset] << 16) | (uint32_t)(data[3 + offset] << 24);
    return value;
}

int32_t LegoinoCommon::ReadInt32LE(uint8_t *data, int offset = 0)
{
    int32_t value = data[0 + offset] | (int16_t)(data[1 + offset] << 8) | (uint32_t)(data[2 + offset] << 16) | (uint32_t)(data[3 + offset] << 24);
    return value;
}

#endif // ESP32