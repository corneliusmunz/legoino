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

#if defined(ESP32)

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

  // This is required to make it working with BLE Scanner and PoweredUp on devices with Android <6.
  // This seems to be not needed for Android >=6
  // TODO: find out why this method helps. Maybe it goes about timeout?
  void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc)
  {
    pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
  };
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
      log_d("message received: %s", msgReceived.c_str());

      // handle port mode information requests and respond dependent on the device type
      if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (byte)MessageType::PORT_MODE_INFORMATION_REQUEST)
      {
        byte port = msgReceived[0x03];
        byte deviceType = _lpf2HubEmulation->getDeviceTypeForPort(port);
        byte mode = msgReceived[0x04];
        byte modeInformationType = msgReceived[0x05];
        std::string payload = _lpf2HubEmulation->getPortModeInformationRequestPayload((DeviceType)deviceType, port, mode, modeInformationType);
        _lpf2HubEmulation->writeValue(MessageType::PORT_MODE_INFORMATION, payload);
      }

      // handle port information requests and respond dependent on the device type
      else if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (byte)MessageType::PORT_INFORMATION_REQUEST)
      {
        byte port = msgReceived[0x03];
        byte deviceType = _lpf2HubEmulation->getDeviceTypeForPort(port);
        byte informationType = msgReceived[0x04];
        std::string payload = _lpf2HubEmulation->getPortInformationPayload((DeviceType)deviceType, port, informationType);
        _lpf2HubEmulation->writeValue(MessageType::PORT_INFORMATION, payload);
      }

      // handle alert response (respond always with status OK)
      else if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (byte)MessageType::HUB_ALERTS)
      {
        if (msgReceived[0x04] == 0x03)
        {
          byte feedback[] = {0x06, 0x00, 0x03, msgReceived[0x03], 0x04, 0x00};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
      }

      // handle hub property requests and respond with the values of the member variables
      else if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::HUB_PROPERTIES && msgReceived[(byte)HubPropertyMessage::OPERATION] == (byte)HubPropertyOperation::REQUEST_UPDATE_DOWNSTREAM)
      {

        if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::ADVERTISING_NAME)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::ADVERTISING_NAME);
          payload.append(_lpf2HubEmulation->getHubName());
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::HW_VERSION)
        {
          byte feedback[] = {0x09, 0x00, 0x01, 0x04, 0x06, 0x00, 0x00, 0x00, 0x01};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::FW_VERSION)
        {
          byte feedback[] = {0x09, 0x00, 0x01, 0x03, 0x06, 0x00, 0x00, 0x02, 0x11};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::RADIO_FIRMWARE_VERSION)
        {
          byte feedback[] = {0x0c, 0x00, 0x01, 0x09, 0x06, 0x32, 0x5f, 0x30, 0x32, 0x5f, 0x30, 0x31};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::MANUFACTURER_NAME)
        {
          byte feedback[] = {0x14, 0x00, 0x01, 0x08, 0x06, 0x4c, 0x45, 0x47, 0x4f, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x20, 0x41, 0x2f, 0x53};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::BATTERY_TYPE)
        {
          byte feedback[] = {0x06, 0x00, 0x01, 0x07, 0x06, (byte)_lpf2HubEmulation->getBatteryType()};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::LEGO_WIRELESS_PROTOCOL_VERSION)
        {
          byte feedback[] = {0x07, 0x00, 0x01, 0x0a, 0x06, 0x00, 0x03};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::PRIMARY_MAC_ADDRESS)
        {
          byte feedback[] = {0x0b, 0x00, 0x01, 0x0d, 0x06, 0x90, 0x84, 0x2b, 0x03, 0x19, 0x7f};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::BUTTON)
        {
          byte feedback[] = {0x06, 0x00, 0x01, 0x02, 0x06, 0x00};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::RSSI)
        {
          byte feedback[] = {0x06, 0x00, 0x01, 0x05, 0x06, 0xc8};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::BATTERY_VOLTAGE)
        {
          byte feedback[] = {0x06, 0x00, 0x01, 0x06, 0x06, 0x47};
          _lpf2HubEmulation->pCharacteristic->setValue(feedback, sizeof(feedback));
          _lpf2HubEmulation->pCharacteristic->notify();
        }
      }
      else if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::HUB_PROPERTIES && msgReceived[(byte)HubPropertyMessage::OPERATION] == (byte)HubPropertyOperation::SET_DOWNSTREAM)
      {
        if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (char)HubPropertyReference::ADVERTISING_NAME)
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
        //Reply to the App "Command excecuted"
        byte msgPortCommandFeedbackReply[] = {0x05, 0x00, 0x82, 0x00, 0x0A};                                           //0x0A Command complete+buffer empty+idle
        msgPortCommandFeedbackReply[(byte)PortOutputMessage::PORT_ID] = msgReceived[(byte)PortOutputMessage::PORT_ID]; //set the port_id
        _lpf2HubEmulation->pCharacteristic->setValue(msgPortCommandFeedbackReply, sizeof(msgPortCommandFeedbackReply));
        _lpf2HubEmulation->pCharacteristic->notify();

        if (msgReceived[(byte)PortOutputMessage::SUB_COMMAND] == 0x51) //OUT_PORT_CMD_WRITE_DIRECT
        {
          if (_lpf2HubEmulation->writePortCallback != nullptr)
          {
            _lpf2HubEmulation->writePortCallback(msgReceived[(byte)PortOutputMessage::PORT_ID], msgReceived[0x07]); //WRITE_DIRECT_VALUE
          }
        }
      }

      if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (byte)MessageType::HUB_ACTIONS && msgReceived[3] == (byte)ActionType::SWITCH_OFF_HUB)
      {
        log_d("disconnect");
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

  Device newDevice = {port, (byte)deviceType};
  connectedDevices[numberOfConnectedDevices] = newDevice;
  numberOfConnectedDevices++;
}

void Lpf2HubEmulation::detachDevice(byte port)
{
  std::string payload;
  payload.push_back((char)port);
  payload.push_back((char)Event::DETACHED_IO);
  writeValue(MessageType::HUB_ATTACHED_IO, payload);

  log_d("port: %x", port);

  bool hasReachedRemovedIndex = false;
  for (int i = 0; i < numberOfConnectedDevices; i++)
  {
    if (hasReachedRemovedIndex)
    {
      connectedDevices[i - 1] = connectedDevices[i];
    }
    if (!hasReachedRemovedIndex && connectedDevices[i].PortNumber == port)
    {
      hasReachedRemovedIndex = true;
    }
  }
  numberOfConnectedDevices--;
}

/**
 * @brief Get the device type of a specific connected device on a defined port in the connectedDevices array
 * @param [in] port number
 * @return device type of the connected device
 */
byte Lpf2HubEmulation::getDeviceTypeForPort(byte portNumber)
{
  log_d("Number of connected devices: %d", numberOfConnectedDevices);
  for (int idx = 0; idx < numberOfConnectedDevices; idx++)
  {
    log_v("device %d, port number: %x, device type: %x, callback address: %x", idx, connectedDevices[idx].PortNumber, connectedDevices[idx].DeviceType, connectedDevices[idx].Callback);
    if (connectedDevices[idx].PortNumber == portNumber)
    {
      log_d("device on port %x has type %x", portNumber, connectedDevices[idx].DeviceType);
      return connectedDevices[idx].DeviceType;
    }
  }
  log_w("no device found for port number %x", portNumber);
  return (byte)DeviceType::UNKNOWNDEVICE;
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

BatteryType Lpf2HubEmulation::getBatteryType()
{
  return _batteryType;
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
  log_d("Starting BLE");

  NimBLEDevice::init(_hubName);
  NimBLEDevice::setPower(ESP_PWR_LVL_N0, ESP_BLE_PWR_TYPE_ADV); // 0dB, Advertisment

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
  _pAdvertising->setMinInterval(32);//0.625ms units -> 20ms
  _pAdvertising->setMaxInterval(64);//0.625ms units -> 40ms

  std::string manufacturerData;
  if (_hubType == HubType::POWERED_UP_HUB)
  {
    log_d("PoweredUp Hub");
    // this is the minimal change that makes PoweredUp working on devices with Android <6
    // https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#last-network-id
    // set Last Network ID to UNKNOWN (0x00)
    const char poweredUpHub[8] = {0x97, 0x03, 0x00, 0x41, 0x07, 0x00, 0x63, 0x00};
    manufacturerData = std::string(poweredUpHub, sizeof(poweredUpHub));
  }
  else if (_hubType == HubType::CONTROL_PLUS_HUB)
  {
    log_d("ControlPlus Hub");
    const char controlPlusHub[8] = {0x97, 0x03, 0x00, 0x80, 0x06, 0x00, 0x41, 0x00};
    manufacturerData = std::string(controlPlusHub, sizeof(controlPlusHub));
  }
  NimBLEAdvertisementData advertisementData = NimBLEAdvertisementData();
  // flags must be present to make PoweredUp working on devices with Android >=6
  // (however it seems to be not needed for devices with Android <6)
  advertisementData.setFlags(BLE_HS_ADV_F_DISC_GEN);
  advertisementData.setManufacturerData(manufacturerData);
  advertisementData.setCompleteServices(NimBLEUUID(LPF2_UUID));
  // scan response data is needed because the uuid128 and manufacturer data takes almost all space in the advertisement data
  // the name is therefore stored in the scan response data
  NimBLEAdvertisementData scanResponseData = NimBLEAdvertisementData();
  scanResponseData.setName(_hubName);
  // set the advertisment flags to 0x06
  scanResponseData.setFlags(BLE_HS_ADV_F_DISC_GEN);
  // set the power level to 0dB
  scanResponseData.addData(std::string{0x02, 0x0A, 0x00});
  // set the slave connection interval range to 20-40ms
  scanResponseData.addData(std::string{0x05, 0x12, 0x10, 0x00, 0x20, 0x00});

  log_d("advertisment data payload(%d): %s", advertisementData.getPayload().length(), advertisementData.getPayload().c_str());
  log_d("scan response data payload(%d): %s", scanResponseData.getPayload().length(), scanResponseData.getPayload().c_str());

  _pAdvertising->setAdvertisementData(advertisementData);
  _pAdvertising->setScanResponseData(scanResponseData);

  log_d("Start advertising");
  NimBLEDevice::startAdvertising();
  log_d("Characteristic defined! Now you can connect with your PoweredUp App!");
}

std::string Lpf2HubEmulation::getPortInformationPayload(DeviceType deviceType, byte port, byte informationType)
{

  std::string payload;
  payload.push_back(port);
  payload.push_back(informationType);

  if (deviceType == DeviceType::TRAIN_MOTOR)
  {
    switch (informationType)
    {
    case 0x01:
      payload.append(std::string{0x01, 0x01, 0x00, 0x00, 0x01, 0x00});
      break;
    case 0x02:
      break;
    default:
      break;
    }
  }
  else if (deviceType == DeviceType::HUB_LED)
  {
    switch (informationType)
    {
    case 0x01:
      payload.append(std::string{0x01, 0x02, 0x00, 0x00, 0x03, 0x00});
      break;
    case 0x02:
      break;
    default:
      break;
    }
  }

  return payload;
}

std::string Lpf2HubEmulation::getPortModeInformationRequestPayload(DeviceType deviceType, byte port, byte mode, byte modeInformationType)
{

  std::string payload;
  payload.push_back(port);
  payload.push_back(mode);
  payload.push_back(modeInformationType);

  if (deviceType == DeviceType::TRAIN_MOTOR)
  {

    if (mode == 0x00)
    {
      switch (modeInformationType)
      {
      case 0x00:
        payload.append(std::string{0x4C, 0x50, 0x46, 0x32, 0x2D, 0x54, 0x52, 0x41, 0x49, 0x4E, 0x00, 0x00});
        break;
      case 0x01:
        payload.append(std::string{0x00, 0x00, 0xC8, 0xC2, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x02:
        payload.append(std::string{0x00, 0x00, 0xC8, 0xC2, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x03:
        payload.append(std::string{0x00, 0x00, 0xC8, 0xC2, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x04:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x05:
        payload.append(std::string{0x00, 0x18});
        break;
      case 0x80:
        payload.append(std::string{0x01, 0x00, 0x04, 0x00});
        break;
      default:
        break;
      }
    }
  }

  if (deviceType == DeviceType::HUB_LED)
  {
    if (mode == 0x00)
    {
      switch (modeInformationType)
      {
      case 0x00:
        payload.append(std::string{0x43, 0x4F, 0x4C, 0x20, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x01:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x41});
        break;
      case 0x02:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x03:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x41});
        break;
      case 0x04:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x05:
        payload.append(std::string{0x00, 0x44});
        break;
      case 0x80:
        payload.append(std::string{0x01, 0x00, 0x01, 0x00});
        break;
      default:
        break;
      }
    }
    else if (mode == 0x01)
    {
      switch (modeInformationType)
      {
      case 0x00:
        payload.append(std::string{0x52, 0x47, 0x42, 0x20, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x01:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x43});
        break;
      case 0x02:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x03:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x43});
        break;
      case 0x04:
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x05:
        payload.append(std::string{0x00, 0x10});
        break;
      case 0x80:
        payload.append(std::string{0x03, 0x00, 0x03, 0x00});
        break;
      default:
        break;
      }
    }
  }

  return payload;
}

#endif // ESP32
