/*
 * Lpf2HubEmulation.cpp - Arduino base class for emulating a hub.
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * 
 * Initial issue/idea by https://github.com/AlbanT
 * Initial implementation idea by https://github.com/marcrupprath
 * Initial implementation with controlling LEDs and outputs by https://github.com/GianCann
 * Many thanks for contributing to that solution!!
 * 
 * Released under MIT License
 * 
*/

#include "Lpf2HubEmulation.h"

class Lpf2HubServerCallbacks : public NimBLEServerCallbacks
{

  Lpf2HubEmulation *_lpf2HubEmulation;

public:
  Lpf2HubServerCallbacks(Lpf2HubEmulation *lpf2HubEmulation) : NimBLEServerCallbacks()
  {
    _lpf2HubEmulation = lpf2HubEmulation;
  }

  void onConnect(NimBLEServer *pServer)
  {
    log_d("Device connected");
    _lpf2HubEmulation->isConnected = true;
  };

  void onDisconnect(NimBLEServer *pServer)
  {
    log_d("Device disconnected");
    _lpf2HubEmulation->isConnected = false;
    _lpf2HubEmulation->isPortInitialized = false;
  }
};

class Lpf2HubCharacteristicCallbacks : public NimBLECharacteristicCallbacks
{

  Lpf2HubEmulation *_lpf2HubEmulation;

public:
  Lpf2HubCharacteristicCallbacks(Lpf2HubEmulation *lpf2HubEmulation) : NimBLECharacteristicCallbacks()
  {
    _lpf2HubEmulation = lpf2HubEmulation;
  }

  void onWrite(NimBLECharacteristic *pCharacteristic)
  {

    std::string msgReceived = pCharacteristic->getValue();

    if (msgReceived.length() > 0)
    {
      log_d("message received: %s", msgReceived);
    
      if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::HUB_PROPERTIES)
      {
        if (msgReceived[0x03] == (char)HubPropertyReference::ADVERTISING_NAME)
        {
          //5..length
          _lpf2HubEmulation->setHubName(msgReceived.substr(5, msgReceived.length() - 5), false);
          log_d("hub name: %s", _lpf2HubEmulation->getHubName().c_str());
        }
      }

      //It's a port out command:
      //execute and send feedback to the App
      if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::PORT_OUTPUT_COMMAND)
      {
        delay(30);

        //Reply to the App "Command excecuted"
        byte msgPortCommandFeedbackReply[] = {0x05, 0x00, 0x82, 0x00, 0x0A}; //0x0A Command complete+buffer empty+idle
        msgPortCommandFeedbackReply[(byte)PortOutputCommand::PORT_ID] = msgReceived[(byte)PortOutputCommand::PORT_ID];         //set the port_id
        _lpf2HubEmulation->pCharacteristic->setValue(msgPortCommandFeedbackReply, sizeof(msgPortCommandFeedbackReply));
        _lpf2HubEmulation->pCharacteristic->notify();

        if (msgReceived[(byte)PortOutputCommand::SUB_COMMAND] == 0x51) //OUT_PORT_CMD_WRITE_DIRECT
        {
          if (_lpf2HubEmulation->writePortCallback != nullptr)
          {
            _lpf2HubEmulation->writePortCallback(msgReceived[(byte)PortOutputCommand::PORT_ID], msgReceived[0x07]); //WRITE_DIRECT_VALUE
          }
        }
      }

      if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (byte)MessageType::HUB_ACTIONS && msgReceived[3] == (byte)ActionType::SWITCH_OFF_HUB)
      {
        log_d("disconnect");
        delay(30);
        byte msgDisconnectionReply[] = {0x04, 0x00, 0x02, 0x31};
        _lpf2HubEmulation->pCharacteristic->setValue(msgDisconnectionReply, sizeof(msgDisconnectionReply));
        _lpf2HubEmulation->pCharacteristic->notify();
        delay(100);
        log_d("restart ESP");
        delay(1000);
        ESP.restart();
      }
    }
  }

  void onRead(NimBLECharacteristic *pCharacteristic)
  {
    log_d("read request");
    uint8_t CharTemp[] = {0x0F, 0x00, 0x04};
    //_lpf2HubEmulation->_pCharacteristic->setValue(CharTemp,3);
  }
};

Lpf2HubEmulation::Lpf2HubEmulation(){};

Lpf2HubEmulation::Lpf2HubEmulation(std::string hubName, HubType hubType)
{
  _hubName = hubName;
  _hubType = hubType;
}

void Lpf2HubEmulation::setWritePortCallback(WritePortCallback callback)
{
  writePortCallback = callback;
}

void Lpf2HubEmulation::attachDevice(byte port, DeviceType deviceType)
{
  std::string payload = "";
  payload.push_back((char)port);
  payload.push_back((char)Event::ATTACHED_IO);
  payload.push_back((char)deviceType);
  std::string versionInformation = {0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10};
  payload.append(versionInformation); //version numbers
  writeValue(MessageType::HUB_ATTACHED_IO, payload);
}

void Lpf2HubEmulation::detachDevice(byte port)
{
  std::string payload;
  payload.push_back((char)port);
  payload.push_back((char)Event::DETACHED_IO);
  writeValue(MessageType::HUB_ATTACHED_IO, payload);
}

void Lpf2HubEmulation::writeValue(MessageType messageType, std::string payload, bool notify)
{
  std::string message = "";
  message.push_back((char)(payload.length() + 3)); // length of message
  message.push_back(0x00);                         // hub id (not used)
  message.push_back((char)messageType);            // message type
  message.append(payload);
  pCharacteristic->setValue(message);

  if (notify)
  {
    pCharacteristic->notify();
  }
  log_d("write message (%d): %s", message.length(), message);
}

void Lpf2HubEmulation::setHubButton(bool pressed)
{
  std::string payload;
  payload.push_back((char)HubPropertyReference::BUTTON);
  payload.push_back((char)(pressed ? ButtonState::PRESSED : ButtonState::RELEASED));
  writeValue(MessageType::HUB_PROPERTIES, payload);
}

void Lpf2HubEmulation::setHubRssi(int8_t rssi)
{
  _rssi = rssi;
  std::string payload;
  payload.push_back((char)HubPropertyReference::RSSI);
  payload.push_back((char)_rssi);
  writeValue(MessageType::HUB_PROPERTIES, payload);
}

void Lpf2HubEmulation::setHubBatteryLevel(uint8_t batteryLevel)
{
  _batteryLevel = batteryLevel;
  std::string payload;
  payload.push_back((char)HubPropertyReference::BATTERY_VOLTAGE);
  payload.push_back((char)_batteryLevel);
  writeValue(MessageType::HUB_PROPERTIES, payload);
}

void Lpf2HubEmulation::setHubBatteryType(BatteryType batteryType)
{
  _batteryType = batteryType;
  std::string payload;
  payload.push_back((char)HubPropertyReference::BATTERY_TYPE);
  payload.push_back((char)_batteryType);
  writeValue(MessageType::HUB_PROPERTIES, payload);
}

void Lpf2HubEmulation::setHubName(std::string hubName, bool notify)
{
  if (hubName.length() > 14)
  {
    _hubName = hubName.substr(0, 14);
  }
  else
  {
    _hubName = hubName;
  }
  if (notify)
  {
    std::string payload;
    payload.push_back((char)HubPropertyReference::ADVERTISING_NAME);
    payload.append(_hubName);
    writeValue(MessageType::HUB_PROPERTIES, payload);
  }
}

std::string Lpf2HubEmulation::getHubName()
{
  return _hubName;
}

void Lpf2HubEmulation::setHubFirmwareVersion(Version version)
{
  _firmwareVersion = version;
}

void Lpf2HubEmulation::setHubHardwareVersion(Version version)
{
  _hardwareVersion = version;
}

void Lpf2HubEmulation::start()
{
  log_d("Starting BLE work!");

  uint8_t newMACAddress[] = {0x91, 0x84, 0x2B, 0x4A, 0x3A, 0x0A};
  esp_base_mac_addr_set(&newMACAddress[0]);
  NimBLEDevice::init(_hubName);

  log_d("Create server");
  _pServer = NimBLEDevice::createServer();
  _pServer->setCallbacks(new Lpf2HubServerCallbacks(this));

  log_d("Create service");
  _pService = _pServer->createService(LPF2_UUID);

  // Create a BLE Characteristic
  pCharacteristic = _pService->createCharacteristic(
      NimBLEUUID(LPF2_CHARACHTERISTIC),
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::NOTIFY |
          NIMBLE_PROPERTY::WRITE_NR);
  // Create a BLE Descriptor and set the callback
  pCharacteristic->setCallbacks(new Lpf2HubCharacteristicCallbacks(this));

  log_d("Service start");

  _pService->start();
  _pAdvertising = NimBLEDevice::getAdvertising();

  _pAdvertising->addServiceUUID(LPF2_UUID);
  _pAdvertising->setScanResponse(true);

  //Techinc HUB
  //const char  ArrManufacturerData[8] = {0x97,0x03,0x00,0x80,0x06,0x00,0x41,0x00};

  //City HUB
  std::string manufacturerData;
  if (_hubType == HubType::POWERED_UP_HUB)
  {
    log_d("PoweredUp Hub");
    const char poweredUpHub[8] = {0x97, 0x03, 0x00, 0x41, 0x07, 0x00, 0x43, 0x00};
    manufacturerData = std::string(poweredUpHub, sizeof(poweredUpHub));
  }
  else if (_hubType == HubType::CONTROL_PLUS_HUB)
  {
    log_d("ControlPlus Hub");
    const char controlPlusHub[8] = {0x97, 0x03, 0x00, 0x80, 0x06, 0x00, 0x41, 0x00};
    manufacturerData = std::string(controlPlusHub, sizeof(controlPlusHub));
  }
  NimBLEAdvertisementData advertisementData = NimBLEAdvertisementData();

  // Not needed because the name is already part of the device
  // if it is added, the max length of 31 bytes is reached and the Service UUID or Manufacturer Data is then missing
  // because of a lenght check in addData to the payload structure
  //  advertisementData.setName("Fake Hub");
  advertisementData.setManufacturerData(manufacturerData);
  advertisementData.setCompleteServices(NimBLEUUID(LPF2_UUID));

  std::string payload = advertisementData.getPayload();
  log_d("advertisment data payload: %s", payload);

  // scan response data is not needed. It could be used to add some more data but it seems that it is not requested by
  // the Lego apps
  _pAdvertising->setAdvertisementData(advertisementData);

  log_d("start advertising");
  NimBLEDevice::startAdvertising();
  log_d("characteristic defined! Now you can read it in your phone!");
}
