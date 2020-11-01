/*
 * Lpf2HubEmulation.h - Arduino base class for emulating a hub.
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * 
 * Initial issue/idea by https://github.com/AlbanT
 * Initial implementation idea by https://github.com/marcrupprath
 * Initial implementation with controlling LEDs and outputs by https://github.com/GianCann
 * 
 * Released under MIT License
 * 
*/

#ifndef Lpf2HubEmulation_h
#define Lpf2HubEmulation_h

#include "Arduino.h"
#include <NimBLEDevice.h>
#include "Lpf2HubConst.h"

typedef void (*WritePortCallback)(byte port, byte value);

struct Device
{
  byte PortNumber;
  byte DeviceType;
};

class Lpf2HubEmulation
{
private:
  // Notification callbacks if values are written to the characteristic
  BLEUUID _bleUuid;
  BLEUUID _charachteristicUuid;
  BLEAddress *_pServerAddress;
  BLEServer *_pServer;
  BLEService *_pService;
  BLEAddress *_hubAddress = nullptr;
  BLEAdvertising *_pAdvertising;

    // Hub information values
  int8_t _rssi;
  uint8_t _batteryLevel;
  BatteryType _batteryType;
  std::string _hubName = "hub";
  HubType _hubType;
  Version _firmwareVersion;
  Version _hardwareVersion;

    // List of connected devices
  Device connectedDevices[13];
  int numberOfConnectedDevices = 0;

public:
  Lpf2HubEmulation();
  Lpf2HubEmulation(std::string hubName, HubType hubType);
  void start();
  void setWritePortCallback(WritePortCallback callback);
  void setHubRssi(int8_t rssi);
  void setHubBatteryLevel(uint8_t batteryLevel);
  void setHubBatteryType(BatteryType batteryType);
  void setHubName(std::string hubName, bool notify = true);


  std::string getHubName();
  BatteryType getBatteryType();

  void setHubFirmwareVersion(Version version);
  void setHubHardwareVersion(Version version);
  void setHubButton(bool pressed);

  void attachDevice(byte port, DeviceType deviceType);
  void detachDevice(byte port);
  byte getDeviceTypeForPort(byte port);

  void writeValue(MessageType messageType, std::string payload, bool notify = true);
  std::string getPortModeInformationRequestPayload(DeviceType deviceType, byte port, byte mode, byte modeInformationType);
  std::string getPortInformationPayload(DeviceType deviceType, byte port, byte informationType);

  bool isConnected = false;
  bool isPortInitialized = false;
  BLECharacteristic *pCharacteristic;
  WritePortCallback writePortCallback = nullptr;

};

#endif