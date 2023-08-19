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
    _lpf2HubEmulation->isSubscripted = false;
    _lpf2HubEmulation->isPortInitialized = false;
  }

  // This is required to make it working with BLE Scanner and PoweredUp on devices with Android <6.
  // This seems to be not needed for Android >=6
  // TODO: find out why this method helps. Maybe it goes about timeout?
  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
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

  void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue)
  {
    log_d("Client subscription status: %s (%d)", 
          subValue == 0 ? "Un-Subscribed" : 
          subValue == 1 ? "Notifications" : 
          subValue == 2 ? "Indications" : 
          subValue == 3 ? "Notifications and Indications" :
          "unknown subscription status",
          subValue);

    _lpf2HubEmulation->isSubscripted = subValue != 0;
  }  

  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    std::string msgReceived = pCharacteristic->getValue();

    if (msgReceived.length() > 0)
    {
      log_d("message received (%d): %s", msgReceived.length(), LegoinoCommon::HexString(msgReceived).c_str());
      log_d("message type: %d", msgReceived[(byte)MessageHeader::MESSAGE_TYPE]);

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
        byte alertType = msgReceived[0x03];
        byte alertOperation = msgReceived[0x04];

        if (alertOperation == 0x03)
        {
          std::string payload;
          payload.push_back((char)alertType);
          payload.push_back(0x04); // Alert Operation, Update (Upstream)
          payload.push_back(0x00); // Alert Payload, Status OK
          _lpf2HubEmulation->writeValue(MessageType::HUB_ALERTS, payload);
        }
      }

      // handle hub property requests and respond with the values of the member variables
      else if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::HUB_PROPERTIES && msgReceived[(byte)HubPropertyMessage::OPERATION] == (byte)HubPropertyOperation::REQUEST_UPDATE_DOWNSTREAM)
      {
        if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::ADVERTISING_NAME)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::ADVERTISING_NAME);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.append(_lpf2HubEmulation->getHubName());
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::HW_VERSION)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::HW_VERSION);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.append(std::string{0x00, 0x00, 0x00, 0x01}); // 0.0.0.1
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::FW_VERSION)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::FW_VERSION);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.append(std::string{0x00, 0x00, 0x02, 0x11});
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::RADIO_FIRMWARE_VERSION)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::RADIO_FIRMWARE_VERSION);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.append(std::string{0x32, 0x5f, 0x30, 0x32, 0x5f, 0x30, 0x31}); // 2_02_01
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::MANUFACTURER_NAME)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::MANUFACTURER_NAME);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);          
          payload.append(std::string{0x4c, 0x45, 0x47, 0x4f, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x20, 0x41, 0x2f, 0x53}); // LEGO System A/S
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::BATTERY_TYPE)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::BATTERY_TYPE);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.push_back((char)_lpf2HubEmulation->getBatteryType());
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::LEGO_WIRELESS_PROTOCOL_VERSION)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::LEGO_WIRELESS_PROTOCOL_VERSION);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.push_back((char)_lpf2HubEmulation->getBatteryType());
          payload.append(std::string{0x00, 0x03}); // 0.3
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::PRIMARY_MAC_ADDRESS)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::PRIMARY_MAC_ADDRESS);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.append(std::string{0x90, 0x84, 0x2b, 0x03, 0x19, 0x7f});
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::BUTTON)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::BUTTON);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.push_back((char)ButtonState::RELEASED);
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::RSSI)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::RSSI);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.push_back(0xc8);
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
        else if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (byte)HubPropertyReference::BATTERY_VOLTAGE)
        {
          std::string payload;
          payload.push_back((char)HubPropertyReference::BATTERY_VOLTAGE);
          payload.push_back((char)HubPropertyOperation::UPDATE_UPSTREAM);
          payload.push_back(0x47); // 71%
          _lpf2HubEmulation->writeValue(MessageType::HUB_PROPERTIES, payload);
        }
      }
      else if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::HUB_PROPERTIES && msgReceived[(byte)HubPropertyMessage::OPERATION] == (byte)HubPropertyOperation::SET_DOWNSTREAM)
      {
        if (msgReceived[(byte)HubPropertyMessage::PROPERTY] == (char)HubPropertyReference::ADVERTISING_NAME)
        {
          // 5..length
          _lpf2HubEmulation->setHubName(msgReceived.substr(5, msgReceived.length() - 5), false);
          log_d("hub name: %s", _lpf2HubEmulation->getHubName().c_str());
        }
      }

      // It's a port out command:
      // execute and send feedback to the App
      if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::PORT_OUTPUT_COMMAND)
      {
        byte port = msgReceived[(byte)PortOutputMessage::PORT_ID];
        byte startCompleteInfo = msgReceived[(byte)PortOutputMessage::STARTUP_AND_COMPLETION];
        byte subCommand = msgReceived[(byte)PortOutputMessage::SUB_COMMAND];

        // Reply to the App "Command excecuted" if the App requests a feedback.
        // https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#port-output-command-feedback-format
        if ((startCompleteInfo & 0x01) != 0) // Command feedback (status) requested
        {
          std::string payload;
          payload.push_back((char)port);
          payload.push_back((byte)PortFeedbackMessage::BUFFER_EMPTY_AND_COMMAND_COMPLETED | (byte)PortFeedbackMessage::IDLE);
          _lpf2HubEmulation->writeValue(MessageType::PORT_OUTPUT_COMMAND_FEEDBACK, payload);
        }

        if (subCommand == 0x51) // OUT_PORT_CMD_WRITE_DIRECT
        {
          byte commandMode = msgReceived[0x06];
          byte power = msgReceived[0x07];
          if (_lpf2HubEmulation->writePortCallback != nullptr)
          {
            _lpf2HubEmulation->writePortCallback(msgReceived[(byte)PortOutputMessage::PORT_ID], power); // WRITE_DIRECT_VALUE
          }
        }
        else if (subCommand == 0x07) // StartSpeed (Speed, MaxPower, UseProfile)
        {
          byte speed = msgReceived[0x06];
          byte maxSpeed = msgReceived[0x07];
          byte useProfile = msgReceived[0x08];
          if (_lpf2HubEmulation->writePortCallback != nullptr)
          {
            _lpf2HubEmulation->writePortCallback(msgReceived[(byte)PortOutputMessage::PORT_ID], speed); // WRITE_DIRECT_VALUE
          }
        }
      }

      if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (byte)MessageType::HUB_ACTIONS && msgReceived[3] == (byte)ActionType::SWITCH_OFF_HUB)
      {
        log_d("disconnect");
        std::string payload;
        payload.push_back(0x31);
        _lpf2HubEmulation->writeValue(MessageType::HUB_ACTIONS, payload);
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
  payload.append(versionInformation); // version numbers
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
    log_v("device %d, port number: %x, device type: %x, callback address: %x", idx, connectedDevices[idx].PortNumber, connectedDevices[idx].DeviceType);
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

  log_d("write message (%d): %s", message.length(), LegoinoCommon::HexString(message).c_str());
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
  _pAdvertising->setMinInterval(32); // 0.625ms units -> 20ms
  _pAdvertising->setMaxInterval(64); // 0.625ms units -> 40ms

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
  // https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#port-information-format
  std::string payload;
  payload.push_back(port);
  payload.push_back(informationType);

  if (deviceType == DeviceType::TRAIN_MOTOR)
  {
    switch (informationType)
    {
    case 0x01:
      // Information Type == MODE INFO (001)
      // Capabilities (Uint8)
      //   Bit 4..7 N/A
      //   Bit 3    Logical Synchronizable
      //   Bit 2    Logical Combinable
      //   Bit 1    Input (seen from Hub)
      //   Bit 0    Output (seen from Hub)
      // Total number of port modes (Uint8)
      // Available Input Port Modes (bitmask) (Uint16)
      // Available Output Port Modes (bitmask) (Uint16)
      // Response: Input (seen from Hub), 1 port mode, 0 input modes, 1 output mode
      payload.append(std::string{0x01, 0x01, 0x00, 0x00, 0x01, 0x00});
      break;
    case 0x02:
      // Information Type == POSSIBLE MODE COMBINATIONS (002)
      // Up to 8 times UWORD bit-fields showing the possible mode/value-sets 
      // combinations for a given sensor. Some combinations cannot be used due to the 
      // need for different H/W setup (mode affects each others, H/W switch- and 
      // settling time etc.)
      // see https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#pos-m
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
      // Information Type == MODE INFO (001)
      // Response: Input (seen from Hub), 2 port modes, 0 input modes, 3 output modes
      payload.append(std::string{0x01, 0x02, 0x00, 0x00, 0x03, 0x00});
      break;
    case 0x02:
      // Information Type == POSSIBLE MODE COMBINATIONS (002)
      break;
    default:
      break;
    }
  }
  else if (deviceType == DeviceType::LIGHT)
  {
    switch (informationType)
    {
    case 0x01:
      // Information Type == MODE INFO (001)
      // Response: Input (seen from Hub), 1 port mode, 0 input modes, 1 output mode
      payload.append(std::string{0x01, 0x01, 0x00, 0x00, 0x01, 0x00});
      break;
    case 0x02:
      // Information Type == POSSIBLE MODE COMBINATIONS (002)
      break;
    default:
      break;
    }
  }

  return payload;
}

std::string Lpf2HubEmulation::getPortModeInformationRequestPayload(DeviceType deviceType, byte port, byte mode, byte modeInformationType)
{
  // https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#port-mode-information-format
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
        // Information Type == NAME (000)
        // The maximum length is 11 chars NOT ASCIIZ terminated - length is decoded from total
        // packet length. ONLY ASCII chars: 0x30.. 0x39, 0x41..0x5A, 0x5F and 0x61..0x7A are
        // allowed.
        // Format: Uint8[0..10]
        // Response: LPF2_TRAIN
        payload.append(std::string{0x4C, 0x50, 0x46, 0x32, 0x5F, 0x54, 0x52, 0x41, 0x49, 0x4E, 0x00});
        break;
      case 0x01:
        // Information Type == RAW (001)
        // The range for the raw (transmitted) signal, remember other ranges are used for scaling
        // the value.
        // Format: RawMin, RawMax [2 * 4 bytes [FLOATING POINT]]
        // Response: -100 - 100
        payload.append(std::string{0x00, 0x00, 0xC8, 0xC2, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x02:
        // Information Type == PCT (002)
        // What % window should the RAW values be scaled to.
        // Format: PctMin, PctMax [2 * 4 bytes [FLOATING POINT]]
        // Response: -100 - 100
        payload.append(std::string{0x00, 0x00, 0xC8, 0xC2, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x03:
        // Information Type == SI (003)
        // As above
        // Format: SiMin, SiMax [2 * 4 bytes [FLOATING POINT]]
        // Response: -100 - 100
        payload.append(std::string{0x00, 0x00, 0xC8, 0xC2, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x04:
        // Information Type == SYMBOL (004)
        // The standard name for a given output.
        // E.g. DEG for degrees. Normally shortened to max. 5 chars.
        // Format: Uint8[0..4]
        // Response: \0\0\0\0\0
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x05:
        // Information Type == MAPPING (005)
        // xxxxxxxx Input side
        // Bit 7  Supports NULL value
        // Bit 6  Supports Functional Mapping 2.0+
        // Bit 5  N/A
        // Bit 4  ABS (Absolute [min..max])
        // Bit 3  REL (Relative [-1..1])
        // Bit 2  DIS (Discrete [0, 1, 2, 3])
        // Bit 1  N/A
        // Bit 0  N/A
        // yyyyyyyy Outpu side
        // Bit 7  Supports NULL value
        // Bit 6  Supports Functional Mapping 2.0+
        // Bit 5  N/A
        // Bit 4  ABS (Absolute [min..max])
        // Bit 3  REL (Relative [-1..1])
        // Bit 2  DIS (Discrete [0, 1, 2, 3])
        // Bit 1  N/A
        // Bit 0  N/A
        // The roles are: The host of the sensor (even a simple and dumb black box) can
        // then decide, what to do with the sensor without any setup (default mode 0 (zero).
        // Using the LSB first (highest priority).
        // Format: Uint16 xxxx xxxx yyyy yyyy
        // Response: 0000000 00011000
        payload.append(std::string{0x00, 0x18});
        break;
      case 0x80:
        // Information Type == VALUE FORMAT (128)
        // Returns the Value Format. No of datasets, type and number of digits.
        // Byte[0]  Number of datasets
        // Byte[1]  Dataset type
        //          00  8 bit
        //          01  16 bit
        //          10  32 bit
        //          11  FLOAT
        // Byte[2]  Total figures
        // Byte[3]  Decimals if any
        // Format: Uint8[4]
        // Response: 1 dataset, 8 bit, 4 figures, 0 decimals
        payload.append(std::string{0x01, 0x00, 0x04, 0x00});
        break;
      case 0x07:
        // Information Type == MOTOR BIAS (007)
        // Initial PWM percentage for a given motor to start running (is of course also depending
        // of use case). 0 - 100 %
        // Format: Uint8
        break;
      case 0x08:
        // Information Type == CAPABILITY BITS (008)
        // Sensor capabilities as bits. A total of 48 bits (6 bytes) can be retrieved from a sensor.
        // For decoding see separate doc. Bytes sent as BIG endianness.
        // Format: Uint8[6]
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
        // Information Type == NAME (000)
        // Response: COL_O
        payload.append(std::string{0x43, 0x4F, 0x4C, 0x5F, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x01:
        // Information Type == RAW (001)
        // Response: 0 - 10
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x41});
        break;
      case 0x02:
        // Information Type == PCT (002)
        // Response: 0 - 100
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x03:
        // Information Type == SI (003)
        // Response: 0 - 10
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x41});
        break;
      case 0x04:
        // Information Type == SYMBOL (004)
        // Response: \0\0\0\0\0
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x05:
        // Information Type == MAPPING (005)
        // Response: 00000000 01000100
        payload.append(std::string{0x00, 0x44});
        break;
      case 0x80:
        // Information Type == VALUE FORMAT (128)
        // Response: 1 dataset, 8 bit, 1 figure, 0 decimals
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
        // Information Type == NAME (000)
        // Response: RGB_0
        payload.append(std::string{0x52, 0x47, 0x42, 0x5F, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x01:
        // Information Type == RAW (001)
        // Response: 0 - 255
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x43});
        break;
      case 0x02:
        // Information Type == PCT (002)
        // Response: 0 - 100
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x42});
        break;
      case 0x03:
        // Information Type == SI (003)
        // Response: 0 - 255
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x43});
        break;
      case 0x04:
        // Information Type == SYMBOL (004)
        // Response: \0\0\0\0\0
        payload.append(std::string{0x00, 0x00, 0x00, 0x00, 0x00});
        break;
      case 0x05:
        // Information Type == MAPPING (005)
        // Response: 00000000 00010000
        payload.append(std::string{0x00, 0x10});
        break;
      case 0x80:
        // Information Type == VALUE FORMAT (128)
        // Response: 3 datasets, 8 bit, 3 figures, 0 decimals
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
