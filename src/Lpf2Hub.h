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
#include "NimBLEDevice.h"
using namespace std::placeholders;
#include "Lpf2HubConst.h"
#include "LegoinoCommon.h"

typedef void (*ButtonCallback)(bool isPressed);
typedef void (*SensorMessageCallback)(byte portNumber, DeviceType deviceType, uint8_t *pData);

typedef struct Device
{
  byte PortNumber;
  byte DeviceType;
  SensorMessageCallback callback;
};

class Lpf2Hub
{
private:
  // Notification callbacks
  ButtonCallback _buttonCallback = nullptr;

public:
  Lpf2Hub();
  void init();
  void init(uint32_t scanDuration);
  void init(std::string deviceAddress);
  void init(std::string deviceAddress, uint32_t scanDuration);
  bool connectHub();
  bool isConnected();
  bool isConnecting();
  NimBLEAddress getHubAddress();

  void setHubName(char name[]);
  void shutDownHub();
  int  getDeviceIndexForPortNumber(byte portNumber);
  byte getDeviceTypeForPortNumber(byte portNumber);
  void setLedColor(Color color);
  void setLedRGBColor(char red, char green, char blue);
  void setLedHSVColor(int hue, double saturation, double value);

  void registerButtonCallback(ButtonCallback buttonCallback);
  void WriteValue(byte command[], int size);

  void parseDeviceInfo(uint8_t *pData);
  void parsePortMessage(uint8_t *pData);
  void parseSensorMessage(uint8_t *pData);
  void parseBoostDistanceAndColor(uint8_t *data);
  void parseBoostTachoMotor(uint8_t *data);
  void parseBoostHubMotor(uint8_t *pData);
  void parseBoostTiltSensor(uint8_t *data);
  void parseControlPlusHubTiltSensor(uint8_t *pData);
  void parsePoweredUpRemote(uint8_t *pData);
  void parsePortAction(uint8_t *pData);
  byte getModeForDeviceType(byte deviceType);
  void registerPortDevice(byte portNumber, byte deviceType);
  void deregisterPortDevice(byte portNumber);
  void activatePortDevice(byte portNumber, byte deviceType, SensorMessageCallback sensorMessageCallback = nullptr);
  void activatePortDevice(byte portNumber, SensorMessageCallback sensorMessageCallback = nullptr);
  void deactivatePortDevice(byte portNumber, byte deviceType);
  void deactivatePortDevice(byte portNumber);
  void activateButtonReports();
  void notifyCallback(NimBLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
  void activateHubUpdates();
  int getTachoMotorRotation();
  double getDistance();
  int getColor();
  int getRssi();
  int getBatteryLevel();
  double getHubVoltage();
  double getHubCurrent();
  int getBoostHubMotorRotation();
  int getTiltX();
  int getTiltY();
  int getTiltZ();
  int getFirmwareVersionBuild();
  int getFirmwareVersionBugfix();
  int getFirmwareVersionMajor();
  int getFirmwareVersionMinor();
  int getHardwareVersionBuild();
  int getHardwareVersionBugfix();
  int getHardwareVersionMajor();
  int getHardwareVersionMinor();
  HubType getHubType();
  bool isButtonPressed();
  bool isLeftRemoteUpButtonPressed();
  bool isLeftRemoteDownButtonPressed();
  bool isLeftRemoteStopButtonPressed();
  bool isLeftRemoteButtonReleased();
  bool isRightRemoteUpButtonPressed();
  bool isRightRemoteDownButtonPressed();
  bool isRightRemoteStopButtonPressed();
  bool isRightRemoteButtonReleased();

  BLEUUID _bleUuid;
  BLEUUID _charachteristicUuid;
  BLEAddress *_pServerAddress;
  BLEAddress *_requestedDeviceAddress = nullptr;
  BLERemoteCharacteristic *_pRemoteCharacteristic;
  boolean _isConnecting;
  boolean _isConnected;
  HubType _hubType;

private:
  Device connectedDevices[13];
  int numberOfConnectedDevices = 0;

  //BLE settings
  uint32_t _scanDuration = 5;
  // Hub information values
  int _lpf2HubRssi;
  uint8_t _lpf2HubBatteryLevel;
  int _lpf2HubHubMotorRotation;
  bool _lpf2HubHubButtonPressed;
  double _lpf2HubVoltage; //V
  double _lpf2HubCurrent; //mA

  int _lpf2HubFirmwareVersionBuild;
  int _lpf2HubFirmwareVersionBugfix;
  int _lpf2HubFirmwareVersionMajor;
  int _lpf2HubFirmwareVersionMinor;

  int _lpf2HubHardwareVersionBuild;
  int _lpf2HubHardwareVersionBugfix;
  int _lpf2HubHardwareVersionMajor;
  int _lpf2HubHardwareVersionMinor;

  // PoweredUp Remote
  bool _lpf2HubRemoteLeftUpButtonPressed;
  bool _lpf2HubRemoteLeftDownButtonPressed;
  bool _lpf2HubRemoteLeftStopButtonPressed;
  bool _lpf2HubRemoteLeftButtonReleased;

  bool _lpf2HubRemoteRightUpButtonPressed;
  bool _lpf2HubRemoteRightDownButtonPressed;
  bool _lpf2HubRemoteRightStopButtonPressed;
  bool _lpf2HubRemoteRightButtonReleased;

  // Hub orientation
  int _lpf2HubTiltX;
  int _lpf2HubTiltY;
  int _lpf2HubTiltZ;
};

#endif