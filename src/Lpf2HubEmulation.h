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
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include "Lpf2HubConst.h"

#define PORT_A 0x00
#define PORT_B 0x01
#define PORT_C 0x02
#define PORT_D 0x03

#define PORT_HUB_LIGHT 0x32

#define PORT_ID 0x03
#define MSG_TYPE 0x02

#define OUT_PORT_SUB_CMD_TYPE 5
#define OUT_PORT_CMD 0x81
#define OUT_PORT_CMD_WRITE_DIRECT 0x51
#define OUT_PORT_FBK 0x082

#define HUB_ACTION_CMD 0x02
#define ACTION_SWITCH_OFF 0x01

#define WRITE_DIRECT_VALUE 0x07

#define SERVICE_UUID        "00001623-1212-EFDE-1623-785FEABCD123"
#define CHARACTERISTIC_UUID "00001624-1212-EFDE-1623-785FEABCD123"


typedef void (*WriteCallback)(byte port, byte deviceType, byte value);

class Lpf2HubEmulation
{
private:
  // Notification callbacks if values are written to the characteristic
  WriteCallback _writeCallback = nullptr;
  BLEUUID _bleUuid;
  BLEUUID _charachteristicUuid;
  BLEAddress *_pServerAddress;
  BLEServer *_pServer;
  BLEService *_pService;
  BLEAddress *_hubAddress = nullptr;
  BLEAdvertising *_pAdvertising;

public:
  Lpf2HubEmulation();
  void start();
  void initializePorts();
  bool isConnected = false;
  bool isPortInitialized = false;
  BLECharacteristic *pCharacteristic;

  // int setBatteryLevel(int batteryLevel);
  // double setHubVoltage(double voltage);
  // double setHubCurrent(double current);
};

#endif