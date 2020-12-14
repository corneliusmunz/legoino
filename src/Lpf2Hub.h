/*
 * Lpf2Hub.h - Arduino base class for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#if defined(ESP32)

#ifndef Lpf2Hub_h
#define Lpf2Hub_h

#include "Arduino.h"
#include "NimBLEDevice.h"
#include "Lpf2HubConst.h"
#include "LegoinoCommon.h"

using namespace std::placeholders;

typedef void (*HubPropertyChangeCallback)(void *hub, HubPropertyReference hubProperty, uint8_t *pData);
typedef void (*PortValueChangeCallback)(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData);

struct Device
{
  byte PortNumber;
  byte DeviceType;
  PortValueChangeCallback Callback;
};

class Lpf2Hub
{

public:
  // constructor
  Lpf2Hub();

  // initializer methods
  void init();
  void init(uint32_t scanDuration);
  void init(std::string deviceAddress);
  void init(std::string deviceAddress, uint32_t scanDuration);

  // hub related methods
  bool connectHub();
  bool isConnected();
  bool isConnecting();
  NimBLEAddress getHubAddress();
  HubType getHubType();
  std::string getHubName();
  void setHubName(char name[]);
  void shutDownHub();
  void activateHubPropertyUpdate(HubPropertyReference hubProperty, HubPropertyChangeCallback hubPropertyChangeCallback = nullptr);
  void deactivateHubPropertyUpdate(HubPropertyReference hubProperty);
  void requestHubPropertyUpdate(HubPropertyReference hubProperty, HubPropertyChangeCallback hubPropertyChangeCallback = nullptr);

  // port and device related methods
  int getDeviceIndexForPortNumber(byte portNumber);
  byte getDeviceTypeForPortNumber(byte portNumber);
  byte getPortForDeviceType(byte deviceType);
  byte getModeForDeviceType(byte deviceType);
  void registerPortDevice(byte portNumber, byte deviceType);
  void deregisterPortDevice(byte portNumber);
  void activatePortDevice(byte portNumber, byte deviceType, PortValueChangeCallback portValueChangeCallback = nullptr);
  void activatePortDevice(byte portNumber, PortValueChangeCallback portValueChangeCallback = nullptr);
  void deactivatePortDevice(byte portNumber, byte deviceType);
  void deactivatePortDevice(byte portNumber);

  // write (set) operations on port devices
  void WriteValue(byte command[], int size);

  void setLedColor(Color color);
  void setLedRGBColor(char red, char green, char blue);
  void setLedHSVColor(int hue, double saturation, double value);

  void stopBasicMotor(byte port);
  void setBasicMotorSpeed(byte port, int speed);

  void setAccelerationProfile(byte port, int16_t time);
  void setDecelerationProfile(byte port, int16_t time);
  void stopTachoMotor(byte port);
  void setTachoMotorSpeed(byte port, int speed, byte maxPower = 100, BrakingStyle brakingStyle = BrakingStyle::BRAKE);
  void setTachoMotorSpeedForTime(byte port, int speed, int16_t time, byte maxPower = 100, BrakingStyle brakingStyle = BrakingStyle::BRAKE);
  void setTachoMotorSpeedForDegrees(byte port, int speed, int32_t degrees, byte maxPower = 100, BrakingStyle brakingStyle = BrakingStyle::BRAKE);
  void setTachoMotorSpeedsForDegrees(int speedLeft, int speedRight, int32_t degrees, byte maxPower = 100, BrakingStyle brakingStyle = BrakingStyle::BRAKE);

  void setAbsoluteMotorPosition(byte port, int speed, int32_t position, byte maxPower = 100, BrakingStyle brakingStyle = BrakingStyle::BRAKE);
  void setAbsoluteMotorEncoderPosition(byte port, int32_t position);

  void playSound(byte sound);
  void playTone(byte number);

  // parse methods to read in the message content of the charachteristic value
  void parseDeviceInfo(uint8_t *pData);
  void parsePortMessage(uint8_t *pData);
  void parseSensorMessage(uint8_t *pData);
  double parseVoltageSensor(uint8_t *pData);
  double parseCurrentSensor(uint8_t *pData);
  double parseDistance(uint8_t *data);
  int parseColor(uint8_t *data);
  int parseReflectivity(uint8_t *pData);
  int parseTachoMotor(uint8_t *data);
  int parseSpeedometer(uint8_t *pData);
  int parseBoostTiltSensorX(uint8_t *data);
  int parseBoostTiltSensorY(uint8_t *data);
  int parseControlPlusHubTiltSensorX(uint8_t *pData);
  int parseControlPlusHubTiltSensorY(uint8_t *pData);
  int parseControlPlusHubTiltSensorZ(uint8_t *pData);
  ButtonState parseRemoteButton(uint8_t *pData);
  void parsePortAction(uint8_t *pData);
  uint8_t parseSystemTypeId(uint8_t *pData);
  byte parseBatteryType(uint8_t *pData);
  uint8_t parseBatteryLevel(uint8_t *pData);
  int parseRssi(uint8_t *pData);
  Version parseVersion(uint8_t *pData);
  ButtonState parseHubButton(uint8_t *pData);
  std::string parseHubAdvertisingName(uint8_t *pData);

  // BLE specific stuff
  void notifyCallback(NimBLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
  BLEUUID _bleUuid;
  BLEUUID _charachteristicUuid;
  BLEAddress *_pServerAddress;
  BLEAddress *_requestedDeviceAddress = nullptr;
  BLERemoteCharacteristic *_pRemoteCharacteristic;
  HubType _hubType;
  std::string _hubName;
  boolean _isConnecting;
  boolean _isConnected;

private:
  // Notification callbacks
  HubPropertyChangeCallback _hubPropertyChangeCallback = nullptr;

  // List of connected devices
  Device connectedDevices[13];
  int numberOfConnectedDevices = 0;

  //BLE settings
  uint32_t _scanDuration = 10;
};

#endif // Lpf2Hub_h

#endif // ESP32
