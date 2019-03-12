/*
 * Lpf2Hub.h - Arduino base class for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#ifndef Lpf2Hub_h
#define Lpf2Hub_h

#include "Arduino.h"
#include "BLEDevice.h"

#define LPF2_UUID "00001623-1212-efde-1623-785feabcd123"
#define LPF2_CHARACHTERISTIC "00001624-1212-efde-1623-785feabcd123"

typedef void (*ButtonCallback)(bool isPressed);

typedef enum HubType
{
  BOOST_MOVE_HUB = 2,
  POWERED_UP_HUB = 3,
  POWERED_UP_REMOTE = 4,
};

typedef enum DeviceType
{
  BASIC_MOTOR = 1,
  TRAIN_MOTOR = 2,
  LED_LIGHTS = 8,
  BOOST_LED = 22,
  BOOST_DISTANCE = 37,
  BOOST_TACHO_MOTOR = 38,
  BOOST_MOVE_HUB_MOTOR = 39,
  BOOST_TILT = 40,
  POWERED_UP_REMOTE_BUTTON = 55
};

typedef enum Color
{
  BLACK = 0,
  PINK = 1,
  PURPLE = 2,
  BLUE = 3,
  LIGHTBLUE = 4,
  CYAN = 5,
  GREEN = 6,
  YELLOW = 7,
  ORANGE = 8,
  RED = 9,
  WHITE = 10,
  NONE = 255
};

class Lpf2Hub
{
private:
  // BLE properties
  HubType _hubType;
  // device properties
  int _rssi = -100;
  int _batteryLevel = 100; //%
  int _voltage = 0;
  int _current = 0;
  // Notification callbacks
  ButtonCallback _buttonCallback = nullptr;

public:
  Lpf2Hub();
  void init();
  bool connectHub();
  bool isConnected();
  bool isConnecting();

  void setHubName(char name[]);
  void shutDownHub();

  void setLedColor(Color color);
  void setLedRGBColor(char red, char green, char blue);

  void registerButtonCallback(ButtonCallback buttonCallback);
  void WriteValue(byte command[], int size);
  byte MapSpeed(int speed);
  static byte *Int16ToByteArray(int16_t x);
  static byte *Int32ToByteArray(int32_t x);
  static unsigned char ReadUInt8(uint8_t *data, int offset);
  static signed char ReadInt8(uint8_t *data, int offset);
  static unsigned short ReadUInt16LE(uint8_t *data, int offset);
  static signed short ReadInt16LE(uint8_t *data, int offset);
  static unsigned int ReadUInt32LE(uint8_t *data, int offset);
  void parseDeviceInfo(uint8_t *pData);
  void parsePortMessage(uint8_t *pData);
  void parseSensorMessage(uint8_t *pData);
  void parsePortAction(uint8_t *pData);
  void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
  void activateHubUpdates();
  BLEUUID _bleUuid;
  BLEUUID _charachteristicUuid;
  BLEAddress *_pServerAddress;
  BLERemoteCharacteristic *_pRemoteCharacteristic;

  boolean _isConnecting;
  boolean _isConnected;
};

#endif