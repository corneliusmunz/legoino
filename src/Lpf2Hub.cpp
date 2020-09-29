/*
 * Lpf2Hub.cpp - Arduino base class for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "Lpf2Hub.h"

// Global variables to store sensor values. Was needed while member function callbacks was 
// not possible with the "old" BLE library. Will be removed in the future
int Lpf2HubTachoMotorRotation;
double Lpf2HubDistance;
int Lpf2HubColor;

/**
 * Derived class which could be added as an instance to the BLEClient for callback handling
 * The current hub is given as a parameter in the constructor to be able to set the 
 * status flags on a disconnect event accordingly
 */
class Lpf2HubClientCallback : public BLEClientCallbacks
{

    Lpf2Hub *_lpf2Hub;

public:
    Lpf2HubClientCallback(Lpf2Hub *lpf2Hub) : BLEClientCallbacks()
    {
        _lpf2Hub = lpf2Hub;
    }

    void onConnect(BLEClient *bleClient)
    {
    }

    void onDisconnect(BLEClient *bleClient)
    {
        _lpf2Hub->_isConnecting = false;
        _lpf2Hub->_isConnected = false;
        LOGLINE("onDisconnect BLECLient event");
    }
};

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class Lpf2HubAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
    Lpf2Hub *_lpf2Hub;

public:
    Lpf2HubAdvertisedDeviceCallbacks(Lpf2Hub *lpf2Hub) : NimBLEAdvertisedDeviceCallbacks()
    {
        _lpf2Hub = lpf2Hub;
    }

    void onResult(NimBLEAdvertisedDevice *advertisedDevice)
    {
        //Found a device, check if the service is contained and optional if address fits requested address
        LOGLINE(advertisedDevice->toString().c_str());

        if (advertisedDevice->haveServiceUUID() && advertisedDevice->getServiceUUID().equals(_lpf2Hub->_bleUuid) && (_lpf2Hub->_requestedDeviceAddress == nullptr || (_lpf2Hub->_requestedDeviceAddress && advertisedDevice->getAddress().equals(*_lpf2Hub->_requestedDeviceAddress))))
        {
            advertisedDevice->getScan()->stop();
            _lpf2Hub->_pServerAddress = new BLEAddress(advertisedDevice->getAddress());

            if (advertisedDevice->haveManufacturerData())
            {
                uint8_t *manufacturerData = (uint8_t *)advertisedDevice->getManufacturerData().data();
                uint8_t manufacturerDataLength = advertisedDevice->getManufacturerData().length();
                if (manufacturerDataLength >= 3)
                {
                    //check device type ID
                    switch (manufacturerData[3])
                    {
                    case DUPLO_TRAIN_HUB_ID:
                        _lpf2Hub->_hubType = HubType::DUPLO_TRAIN_HUB;
                        LOGLINE("Hubtype: DUPLO_TRAIN_HUB");
                        break;
                    case BOOST_MOVE_HUB_ID:
                        _lpf2Hub->_hubType = HubType::BOOST_MOVE_HUB;
                        LOGLINE("Hubtype: BOOST_MOVE_HUB");
                        break;
                    case POWERED_UP_HUB_ID:
                        _lpf2Hub->_hubType = HubType::POWERED_UP_HUB;
                        LOGLINE("Hubtype: POWERED_UP_HUB");
                        break;
                    case POWERED_UP_REMOTE_ID:
                        _lpf2Hub->_hubType = HubType::POWERED_UP_REMOTE;
                        LOGLINE("Hubtype: POWERED_UP_REMOTE");
                        break;
                    case CONTROL_PLUS_HUB_ID:
                        _lpf2Hub->_hubType = HubType::CONTROL_PLUS_HUB;
                        LOGLINE("Hubtype: CONTROL_PLUS_HUB");
                        break;
                    default:
                        _lpf2Hub->_hubType = HubType::UNKNOWNHUB;
                        LOGLINE("Hubtype: UNKNOWN");
                        break;
                    }
                }
            }
            _lpf2Hub->_isConnecting = true;
        }
    }
};

/**
 * @brief Write value to the remote characteristic
 * @param [in] command byte array which contains the ble command
 * @param [in] size length of the command byte array
 */
void Lpf2Hub::WriteValue(byte command[], int size)
{
    byte commandWithCommonHeader[size + 2] = {size + 2, 0x00};
    memcpy(commandWithCommonHeader + 2, command, size);
    _pRemoteCharacteristic->writeValue(commandWithCommonHeader, sizeof(commandWithCommonHeader), false);
}

/**
 * @brief Register a device on a defined port. This will store the device
 * in the connectedDevices array. This method will be called if a port connection
 * event is triggered by the hub
 * 
 * @param [in] port number where the device is connected
 * @param [in] device type of the connected device
 */
void Lpf2Hub::registerPortDevice(byte portNumber, byte deviceType)
{
    LOG("registerPortDevice Port:");
    LOG(portNumber, HEX);
    LOG(" DeviceType:");
    LOGLINE(deviceType, HEX);

    Device newDevice = {portNumber, deviceType, nullptr};
    connectedDevices[numberOfConnectedDevices] = newDevice;
    numberOfConnectedDevices++;
}

/**
 * @brief Remove a device from the connectedDevices array. This method
 * will be called if a port disconnection event is triggered by the hub
 * 
 * @param [in] port number where the device is connected
 */
void Lpf2Hub::deregisterPortDevice(byte portNumber)
{
    LOG("deregisterPortDevice Port:");
    LOGLINE(portNumber, HEX);

    bool hasReachedRemovedIndex = false;
    for (int i = 0; i < numberOfConnectedDevices; i++)
    {
        if (hasReachedRemovedIndex)
        {
            connectedDevices[i - 1] = connectedDevices[i];
        }
        if (!hasReachedRemovedIndex && connectedDevices[i].PortNumber == portNumber)
        {
            hasReachedRemovedIndex = true;
        }
    }
    numberOfConnectedDevices--;
}

/**
 * @brief Activate device for receiving updates. E.g. activate a color/distance sensor to
 * write updates on the characteristic if a value has changed. An optional callback could be
 * regeistered here. This function will be called if the update event will occur. 
 * 
 * @param [in] port number where the device is connected
 * @param [in] callback function which will be called on an update event
 */
void Lpf2Hub::activatePortDevice(byte portNumber, SensorMessageCallback sensorMessageCallback)
{
    LOGLINE("activatePortDevice(portNumber)");
    byte deviceType = getDeviceTypeForPortNumber(portNumber);
    activatePortDevice(portNumber, deviceType, sensorMessageCallback);
}

/**
 * @brief Activate device for receiving updates. E.g. activate a color/distance sensor to
 * write updates on the characteristic if a value has changed. An optional callback could be
 * regeistered here. This function will be called if the update event will occur. The Update mode 
 * is currently fixed based on the device type
 * 
 * @param [in] port number where the device is connected
 * @param [in] deviceType of the connected port
 * @param [in] callback function which will be called on an update event
 */
void Lpf2Hub::activatePortDevice(byte portNumber, byte deviceType, SensorMessageCallback sensorMessageCallback)
{
    LOGLINE("activatePortDevice");
    byte mode = getModeForDeviceType(deviceType);
    LOG("mode for device: ");
    LOGLINE(mode, HEX);
    int deviceIndex = getDeviceIndexForPortNumber(portNumber);
    connectedDevices[deviceIndex].callback = sensorMessageCallback;
    byte activatePortDeviceMessage[8] = {0x41, portNumber, mode, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(activatePortDeviceMessage, 8);
}

/**
 * @brief Deactivate device for receiving updates. 
 * 
 * @param [in] port number where the device is connected
 */
void Lpf2Hub::deactivatePortDevice(byte portNumber)
{
    byte deviceType = getDeviceTypeForPortNumber(portNumber);
    deactivatePortDevice(portNumber, deviceType);
}

/**
 * @brief Deactivate device for receiving updates. 
 * 
 * @param [in] port number where the device is connected
 * @param [in] device type 
 */
void Lpf2Hub::deactivatePortDevice(byte portNumber, byte deviceType)
{
    byte mode = getModeForDeviceType(deviceType);
    byte deactivatePortDeviceMessage[8] = {0x41, portNumber, mode, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(deactivatePortDeviceMessage, 8);
}

/**
 * @brief Activate updates of hub button events. 
 * 
 */
void Lpf2Hub::activateButtonReports()
{
    LOGLINE("Activate Button Reports");
    byte activateButtonReportsMessage[3] = {0x01, 0x02, 0x02};
    WriteValue(activateButtonReportsMessage, 3);
}

/**
 * @brief Parse the incoming characteristic notification for a Device Info Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parseDeviceInfo(uint8_t *pData)
{
    LOGLINE("parseDeviceInfo");
    // Advertising name
    if (pData[3] == 0x01)
    {
        int charArrayLength = min(pData[0] - 5, 14);
        char name[charArrayLength + 1];
        for (int i; i < charArrayLength; i++)
        {
            name[i] = pData[5 + i];
        }
        name[charArrayLength + 1] = 0;
        LOG("device name: ");
        LOG(name);
        LOGLINE();
    }
    // Button press reports
    else if (pData[3] == 0x02)
    {
        if (pData[5] == 1)
        {
            if (_buttonCallback != nullptr)
            {
                _buttonCallback(true);
            }
            LOGLINE("button PRESSED");
            _lpf2HubHubButtonPressed = true;
            return;
        }
        else if (pData[5] == 0)
        {
            if (_buttonCallback != nullptr)
            {
                _buttonCallback(false);
            }
            LOGLINE("button RELEASED");
            _lpf2HubHubButtonPressed = false;
            return;
        }
    }
    else if (pData[3] == 0x03) // Firmware version
    {
        _lpf2HubFirmwareVersionBuild = LegoinoCommon::ReadUInt16LE(pData, 5);
        _lpf2HubFirmwareVersionBugfix = LegoinoCommon::ReadUInt8(pData, 7);
        _lpf2HubFirmwareVersionMajor = LegoinoCommon::ReadUInt8(pData, 8) >> 4;
        _lpf2HubFirmwareVersionMinor = LegoinoCommon::ReadUInt8(pData, 8) & 0xf;

        LOG("Firmware version major:");
        LOG(_lpf2HubFirmwareVersionMajor);
        LOG(" minor:");
        LOG(_lpf2HubFirmwareVersionMinor);
        LOG(" bugfix:");
        LOG(_lpf2HubFirmwareVersionBugfix);
        LOG(" build:");
        LOG(_lpf2HubFirmwareVersionBuild);
        LOGLINE();
    }
    else if (pData[3] == 0x04) // Hardware version
    {
        _lpf2HubHardwareVersionBuild = LegoinoCommon::ReadUInt16LE(pData, 5);
        _lpf2HubHardwareVersionBugfix = LegoinoCommon::ReadUInt8(pData, 7);
        _lpf2HubHardwareVersionMajor = LegoinoCommon::ReadUInt8(pData, 8) >> 4;
        _lpf2HubHardwareVersionMinor = LegoinoCommon::ReadUInt8(pData, 8) & 0xf;

        LOG("Hardware version major:");
        LOG(_lpf2HubHardwareVersionMajor);
        LOG(" minor:");
        LOG(_lpf2HubHardwareVersionMinor);
        LOG(" bugfix:");
        LOG(_lpf2HubHardwareVersionBugfix);
        LOG(" build:");
        LOG(_lpf2HubHardwareVersionBuild);
        LOGLINE();
    }
    else if (pData[3] == 0x05) // RSSI
    {
        LOG("RSSI update: ");
        _lpf2HubRssi = LegoinoCommon::ReadInt8(pData, 5);
        LOG(_lpf2HubRssi);
        LOGLINE();
    }
    else if (pData[3] == 0x06) // Battery level reports
    {
        _lpf2HubBatteryLevel = LegoinoCommon::ReadUInt8(pData, 5);
        LOG("Battery level: ");
        LOG(_lpf2HubBatteryLevel);
        LOG("%");
        LOGLINE();
    }
    else if (pData[3] == 0x07) // Battery type
    {
        LOG("Battery type: ");
        if (pData[5] == 0x00)
        {
            LOG("Normal");
        }
        else if (pData[5] == 0x01)
        {
            LOG("Recharchable");
        }
        LOGLINE();
    }
    else if (pData[3] == 0x0B) // System type ID
    {
        LOG("System type ID: ");
        uint8_t typeId = LegoinoCommon::ReadUInt8(pData, 5);
        LOG(typeId);
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Port Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parsePortMessage(uint8_t *pData)
{
    LOGLINE("parsePortMessage");
    byte port = pData[3];
    bool isConnected = (pData[4] == 1 || pData[4] == 2) ? true : false;
    LOG("Port ");
    LOG(port, HEX);
    if (isConnected)
    {
        LOG(" is connected with device ");
        LOGLINE(pData[5], DEC);
        registerPortDevice(port, pData[5]);
    }
    else
    {
        LOGLINE(" is disconnected");
        deregisterPortDevice(port);
    }
}

int Lpf2Hub::parseBoostTiltSensorX(uint8_t *pData)
{
    LOGLINE("parseBoostTiltSensorX");
    int value = LegoinoCommon::ReadInt8(pData, 4);
    LOG("x: ");
    LOG(value, DEC);
    return value;
}

int Lpf2Hub::parseBoostTiltSensorY(uint8_t *pData)
{
    LOGLINE("parseBoostTiltSensorY");
    int value = LegoinoCommon::ReadInt8(pData, 5);
    LOG("y: ");
    LOG(value, DEC);
    return value;
}

int Lpf2Hub::parseControlPlusHubTiltSensorX(uint8_t *pData)
{
    LOGLINE("parseControlPlusTiltSensorX");
    int value = LegoinoCommon::ReadInt16LE(pData, 4);
    LOG("x: ");
    LOG(value, DEC);
    return value;
}

int Lpf2Hub::parseControlPlusHubTiltSensorY(uint8_t *pData)
{
    LOGLINE("parseControlPlusTiltSensorY");
    int value = LegoinoCommon::ReadInt16LE(pData, 6);
    LOG("y: ");
    LOG(value, DEC);
    return value;
}

int Lpf2Hub::parseControlPlusHubTiltSensorZ(uint8_t *pData)
{
    LOGLINE("parseControlPlusTiltSensorZ");
    int value = LegoinoCommon::ReadInt16LE(pData, 8);
    LOG("z: ");
    LOG(value, DEC);
    return value;
}

double Lpf2Hub::parseCurrentSensor(uint8_t *pData)
{
    int currentRaw = LegoinoCommon::ReadUInt16LE(pData, 4);
    double current = (double)currentRaw * LPF2_CURRENT_MAX / LPF2_CURRENT_MAX_RAW;
    LOG("Current value [mA]: ");
    LOGLINE(current);
    return current;
}

double Lpf2Hub::parseVoltageSensor(uint8_t *pData)
{
    int voltageRaw = LegoinoCommon::ReadUInt16LE(pData, 4);
    double voltage = (double)voltageRaw * LPF2_VOLTAGE_MAX / LPF2_VOLTAGE_MAX_RAW;
    LOG("Hub Voltage : ");
    LOGLINE(voltage);
    LOGLINE();
    return voltage;
}

int Lpf2Hub::parseTachoMotor(uint8_t *pData)
{
    LOGLINE("parseBoostTachoMotor");
    int value = LegoinoCommon::ReadInt32LE(pData, 4);
    LOG("Tacho motor rotation: ");
    LOGLINE(value, DEC);
    return value;
}

double Lpf2Hub::parseDistance(uint8_t *pData)
{
    LOGLINE("parseDistance");
    int partial = pData[7];
    double distance = (double)pData[5];
    if (partial > 0)
    {
        distance += 1.0 / partial;
    }
    distance = floor(distance * 25.4) - 20.0;

    LOG("Distance: ");
    LOG(distance, DEC);
    return distance;
}

int Lpf2Hub::parseColor(uint8_t *pData)
{
    LOGLINE("parseColor");
    int color = pData[4];
    LOG(" Color: ");
    if (Lpf2HubColor > 10)
    {
        LOGLINE("undefined");
    }
    else
    {
        LOGLINE(COLOR_STRING[Lpf2HubColor]);
    }
    return color;
}

ButtonState Lpf2Hub::parseButton(uint8_t *pData)
{
    LOGLINE("parse Remote Button");
    int buttonState = pData[4];
    LOG("ButtonState: ");
    LOGLINE(buttonState, HEX);
    return (ButtonState)buttonState;
}

byte Lpf2Hub::getModeForDeviceType(byte deviceType)
{
    switch (deviceType)
    {
    case (byte)DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
        return 0x02;
    case (byte)DeviceType::TRAIN_MOTOR:
        return 0x02;
    case (byte)DeviceType::MEDIUM_LINEAR_MOTOR:
        return 0x02;
    case (byte)DeviceType::MOVE_HUB_MEDIUM_LINEAR_MOTOR:
        return 0x02;
    case (byte)DeviceType::COLOR_DISTANCE_SENSOR:
        return 0x08;
    case (byte)DeviceType::MOVE_HUB_TILT_SENSOR:
        return 0x00;
    case (byte)DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR:
        return 0x02;
    case (byte)DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR:
        return 0x02;
    default:
        return 0x00;
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Sensor Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parseSensorMessage(uint8_t *pData)
{
    LOGLINE("parseSensorMessage");
    int deviceIndex = getDeviceIndexForPortNumber(pData[3]);

    byte deviceType = connectedDevices[deviceIndex].DeviceType;

    if (connectedDevices[deviceIndex].callback != nullptr)
    {
        connectedDevices[deviceIndex].callback(pData[3], (DeviceType)deviceType, pData);
    }

    if (deviceType == (byte)DeviceType::CURRENT_SENSOR)
    {
        _lpf2HubCurrent = parseCurrentSensor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::VOLTAGE_SENSOR)
    {
        _lpf2HubVoltage = parseVoltageSensor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::MEDIUM_LINEAR_MOTOR || deviceType == (byte)DeviceType::MOVE_HUB_MEDIUM_LINEAR_MOTOR)
    {
        Lpf2HubTachoMotorRotation = parseTachoMotor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::COLOR_DISTANCE_SENSOR)
    {
        Lpf2HubDistance = parseDistance(pData);
        Lpf2HubColor = parseColor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::MOVE_HUB_TILT_SENSOR)
    {
        _lpf2HubTiltX = parseBoostTiltSensorX(pData);
        _lpf2HubTiltY = parseBoostTiltSensorY(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::TECHNIC_MEDIUM_HUB_TILT_SENSOR)
    {
        _lpf2HubTiltX = parseControlPlusHubTiltSensorX(pData);
        _lpf2HubTiltY = parseControlPlusHubTiltSensorY(pData);
        _lpf2HubTiltZ = parseControlPlusHubTiltSensorZ(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::REMOTE_CONTROL_BUTTON)
    {
        int port = pData[3];
        LOG("Port: ");
        LOG(port, DEC);
        int buttonState = (int)parseButton(pData);
        if (buttonState == 0x01)
        {
            LOGLINE(" ButtonState: UP");
            if (port == 0x00)
            {
                _lpf2HubRemoteLeftUpButtonPressed = true;
            }
            else if (port == 0x01)
            {
                _lpf2HubRemoteRightUpButtonPressed = true;
            }
        }
        else if (buttonState == 0xff)
        {
            LOGLINE(" ButtonState: DOWN");
            if (port == 0x00)
            {
                _lpf2HubRemoteLeftDownButtonPressed = true;
            }
            else if (port == 0x01)
            {
                _lpf2HubRemoteRightDownButtonPressed = true;
            }
        }
        else if (buttonState == 0x7f)
        {
            LOGLINE(" ButtonState: STOP");
            if (port == 0x00)
            {
                _lpf2HubRemoteLeftStopButtonPressed = true;
            }
            else if (port == 0x01)
            {
                _lpf2HubRemoteRightStopButtonPressed = true;
            }
        }
        else if (buttonState == 0x00)
        {
            LOGLINE(" ButtonState: RELEASED");
            if (port == 0x00)
            {
                _lpf2HubRemoteLeftUpButtonPressed = false;
                _lpf2HubRemoteLeftDownButtonPressed = false;
                _lpf2HubRemoteLeftStopButtonPressed = false;
                _lpf2HubRemoteLeftButtonReleased = true;
            }
            else if (port == 0x01)
            {
                _lpf2HubRemoteRightUpButtonPressed = false;
                _lpf2HubRemoteRightDownButtonPressed = false;
                _lpf2HubRemoteRightStopButtonPressed = false;
                _lpf2HubRemoteRightButtonReleased = true;
            }
        }
        return;
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Port Action Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parsePortAction(uint8_t *pData)
{
    LOGLINE("parsePortAction");
}

/**
 * @brief Callback function for notifications of a specific characteristic
 * @param [in] pBLERemoteCharacteristic The pointer to the characteristic
 * @param [in] pData The pointer to the received data
 * @param [in] length The length of the data array
 * @param [in] isNotify 
 */
void Lpf2Hub::notifyCallback(
    NimBLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    LOG("Notify callback for characteristic ");
    LOG(pBLERemoteCharacteristic->getUUID().toString().c_str());
    LOG("data: ");

    for (int i = 0; i < length; i++)
    {
        LOG(pData[i], HEX);
        LOG(" ");
    }
    LOGLINE();

    switch (pData[2])
    {
    case 0x01:
    {
        parseDeviceInfo(pData);
        break;
    }
    case 0x04:
    {
        parsePortMessage(pData);
        break;
    }
    case 0x45:
    {
        parseSensorMessage(pData);
        break;
    }
    case 0x82:
    {
        parsePortAction(pData);
        break;
    }
    }
}

Lpf2Hub::Lpf2Hub(){};

/**
 * @brief Init function set the UUIDs and scan for the Hub
 */
void Lpf2Hub::init()
{
    _isConnected = false;
    _isConnecting = false;
    _bleUuid = BLEUUID(LPF2_UUID);
    _charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
    _hubType = HubType::UNKNOWNHUB;

    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();

    pBLEScan->setAdvertisedDeviceCallbacks(new Lpf2HubAdvertisedDeviceCallbacks(this));

    pBLEScan->setActiveScan(true);
    pBLEScan->start(_scanDuration);
}

/**
 * @brief Init function set the UUIDs and scan for the Hub
 * @param [in] deviceAddress to which the arduino should connect represented by a hex string of the format: 00:00:00:00:00:00
 */
void Lpf2Hub::init(std::string deviceAddress)
{
    _requestedDeviceAddress = new BLEAddress(deviceAddress);
    init();
}

void Lpf2Hub::init(uint32_t scanDuration)
{
    _scanDuration = scanDuration;
    init();
}

void Lpf2Hub::init(std::string deviceAddress, uint32_t scanDuration)
{
    _requestedDeviceAddress = new BLEAddress(deviceAddress);
    _scanDuration = scanDuration;
    init();
}

NimBLEAddress Lpf2Hub::getHubAddress()
{
    NimBLEAddress pAddress = *_pServerAddress;
    return pAddress;
}

int Lpf2Hub::getDeviceIndexForPortNumber(byte portNumber)
{
    LOGLINE("getDeviceIndexForPortNumber");
    LOGLINE(numberOfConnectedDevices, DEC);
    for (int idx = 0; idx < numberOfConnectedDevices; idx++)
    {
        LOGLINE(idx, DEC);
        LOGLINE(connectedDevices[idx].PortNumber, HEX);
        LOGLINE(connectedDevices[idx].DeviceType, HEX);
        LOGLINE((long)(connectedDevices[idx].callback), HEX);
        if (connectedDevices[idx].PortNumber == portNumber)
        {
            return idx;
        }
    }

    //ToDo: What happens if the device could not be found
}

byte Lpf2Hub::getDeviceTypeForPortNumber(byte portNumber)
{
    LOGLINE("getDeviceTypeForPortNumber");
    LOGLINE(numberOfConnectedDevices, DEC);
    for (int idx = 0; idx < numberOfConnectedDevices; idx++)
    {
        LOGLINE(idx, DEC);
        LOGLINE(connectedDevices[idx].PortNumber, HEX);
        LOGLINE(connectedDevices[idx].DeviceType, HEX);
        if (connectedDevices[idx].PortNumber == portNumber)
        {
            LOG("deviceType: ");
            LOGLINE(connectedDevices[idx].DeviceType, HEX);
            return connectedDevices[idx].DeviceType;
        }
    }

    return (byte)DeviceType::UNKNOWNDEVICE;
}

/**
 * @brief Register the callback function if a button message is received
 * @param [in] buttonCallback Function pointer to the callback function which handles the button notification
 */
void Lpf2Hub::registerButtonCallback(ButtonCallback buttonCallback)
{
    _buttonCallback = buttonCallback;
}

/**
 * @brief Set the color of the HUB LED with predefined colors
 * @param [in] color one of the available hub colors
 */
void Lpf2Hub::setLedColor(Color color)
{
    byte setColorMode[8] = {0x41, 0x32, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(setColorMode, 8);
    byte setColor[6] = {0x81, 0x32, 0x11, 0x51, 0x00, color};
    WriteValue(setColor, 6);
}

/**
 * @brief Set the color of the HUB LED with RGB values 
 * @param [in] red 0..255 
 * @param [in] green 0..255 
 * @param [in] blue 0..255 
 */
void Lpf2Hub::setLedRGBColor(char red, char green, char blue)
{
    byte setRGBMode[8] = {0x41, 0x32, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(setRGBMode, 8);
    byte setRGBColor[8] = {0x81, 0x32, 0x11, 0x51, 0x01, red, green, blue};
    WriteValue(setRGBColor, 8);
}

/**
 * @brief Set the color of the HUB LED with HSV values 
 * @param [in] hue 0..360 
 * @param [in] saturation 0..1 
 * @param [in] value 0..1
 */
void Lpf2Hub::setLedHSVColor(int hue, double saturation, double value)
{
    hue = hue % 360; // map hue to 0..360
    double huePart = hue / 60.0;
    double fract = huePart - floor(huePart);

    double p = value * (1. - saturation);
    double q = value * (1. - saturation * fract);
    double t = value * (1. - saturation * (1. - fract));

    if (huePart >= 0.0 && huePart < 1.0)
    {
        setLedRGBColor((char)(value * 255), (char)(t * 255), (char)(p * 255));
    }
    else if (huePart >= 1.0 && huePart < 2.0)
    {
        setLedRGBColor((char)(q * 255), (char)(value * 255), (char)(p * 255));
    }
    else if (huePart >= 2.0 && huePart < 3.0)
    {
        setLedRGBColor((char)(p * 255), (char)(value * 255), (char)(t * 255));
    }
    else if (huePart >= 3.0 && huePart < 4.0)
    {
        setLedRGBColor((char)(p * 255), (char)(q * 255), (char)(value * 255));
    }
    else if (huePart >= 4.0 && huePart < 5.0)
    {
        setLedRGBColor((char)(t * 255), (char)(p * 255), (char)(value * 255));
    }
    else if (huePart >= 5.0 && huePart < 6.0)
    {
        setLedRGBColor((char)(value * 255), (char)(p * 255), (char)(q * 255));
    }
    else
    {
        setLedRGBColor(0, 0, 0);
    }
}

/**
 * @brief Send the Shutdown command to the HUB
 */
void Lpf2Hub::shutDownHub()
{
    byte shutdownCommand[2] = {0x02, 0X01};
    WriteValue(shutdownCommand, 2);
}

/**
 * @brief Set name of the HUB
 * @param [in] name character array which contains the name (max 14 characters are supported)
 */
void Lpf2Hub::setHubName(char name[])
{
    int nameLength = strlen(name);
    if (nameLength > 14)
    {
        return;
    }

    char offset = 3;
    int arraySize = offset + nameLength;
    byte setNameCommand[arraySize] = {0x01, 0x01, 0x01};

    memcpy(setNameCommand + offset, name, nameLength);
    WriteValue(setNameCommand, arraySize);
}

void Lpf2Hub::activateHubUpdates()
{
    // Activate reports
    byte setButtonCommand[3] = {0x01, 0x02, 0x02};
    WriteValue(setButtonCommand, 3);

    byte setBatteryLevelCommand[3] = {0x01, 0x06, 0x02};
    WriteValue(setBatteryLevelCommand, 3);

    byte setBatteryTypeCommand[3] = {0x01, 0x07, 0x02};
    WriteValue(setBatteryTypeCommand, 3);

    byte setRSSICommand[3] = {0x01, 0x05, 0x02};
    WriteValue(setRSSICommand, 3);

    byte setCurrentReport[8] = {0x41, 0x3b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(setCurrentReport, 8);

    byte setVoltageReport[8] = {0x41, 0x3c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(setVoltageReport, 8);

    byte setFWCommand[3] = {0x01, 0x03, 0x05};
    WriteValue(setFWCommand, 3);

    byte setHWCommand[3] = {0x01, 0x04, 0x05};
    WriteValue(setHWCommand, 3);
}

/**
 * @brief Connect to the HUB, get a reference to the characteristic and register for notifications
 */
bool Lpf2Hub::connectHub()
{
    BLEAddress pAddress = *_pServerAddress;
    NimBLEClient *pClient = nullptr;

    Serial.print("Number of Clients: ");
    Serial.println(NimBLEDevice::getClientListSize(), DEC);

    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getClientListSize())
    {
        /** Special case when we already know this device, we send false as the 
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(pAddress);
        if (pClient)
        {
            if (!pClient->connect(pAddress, false))
            {
                Serial.println("Reconnect failed");
                return false;
            }
            Serial.println("Reconnected client");
        }
        /** We don't already have a client that knows this device,
         *  we will check for a client that is disconnected that we can use.
         */
        else
        {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient)
    {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
        {
            Serial.print("Max clients reached - no more connections available: ");
            Serial.println(NimBLEDevice::getClientListSize(), DEC);
            return false;
        }

        pClient = NimBLEDevice::createClient();
        Serial.println("New client created");
    }

    if (!pClient->isConnected())
    {
        if (!pClient->connect(pAddress))
        {
            Serial.println("Failed to connect");
            return false;
        }
    }

    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(pClient->getRssi());

    LOGLINE("get pClient");
    BLERemoteService *pRemoteService = pClient->getService(_bleUuid);
    if (pRemoteService == nullptr)
    {
        LOGLINE("get pClient failed");
        return false;
    }
    LOGLINE("get pRemoteService");

    _pRemoteCharacteristic = pRemoteService->getCharacteristic(_charachteristicUuid);
    if (_pRemoteCharacteristic == nullptr)
    {
        LOGLINE("get pRemoteService failed");

        return false;
    }

    // register notifications (callback function) for the characteristic
    if (_pRemoteCharacteristic->canNotify())
    {
        _pRemoteCharacteristic->subscribe(true, std::bind(&Lpf2Hub::notifyCallback, this, _1, _2, _3, _4), true);
    }

    // add callback instance to get notified if a disconnect event appears
    pClient->setClientCallbacks(new Lpf2HubClientCallback(this));

    // Set states
    _isConnected = true;
    _isConnecting = false;
    return true;
}

/**
 * @brief Retrieve the connection state. The BLE client (ESP32) has found a service with the desired UUID (HUB)
 * If this state is available, you can try to connect to the Hub
 */
bool Lpf2Hub::isConnecting()
{
    return _isConnecting;
}

/**
 * @brief Retrieve the connection state. The BLE client (ESP32) is connected to the server (HUB)
 */
bool Lpf2Hub::isConnected()
{
    return _isConnected;
}

int Lpf2Hub::getColor()
{
    return Lpf2HubColor;
}

double Lpf2Hub::getDistance()
{
    return Lpf2HubDistance;
}

int Lpf2Hub::getTachoMotorRotation()
{
    return Lpf2HubTachoMotorRotation;
}

int Lpf2Hub::getBoostHubMotorRotation()
{
    return _lpf2HubHubMotorRotation;
}

int Lpf2Hub::getRssi()
{
    return _lpf2HubRssi;
}

int Lpf2Hub::getBatteryLevel()
{
    return _lpf2HubBatteryLevel;
}

double Lpf2Hub::getHubVoltage()
{
    return _lpf2HubVoltage;
}

double Lpf2Hub::getHubCurrent()
{
    return _lpf2HubCurrent;
}

int Lpf2Hub::getTiltX()
{
    return _lpf2HubTiltX;
}

int Lpf2Hub::getTiltY()
{
    return _lpf2HubTiltY;
}

int Lpf2Hub::getTiltZ()
{
    return _lpf2HubTiltZ;
}

int Lpf2Hub::getFirmwareVersionBuild()
{
    return _lpf2HubFirmwareVersionBuild;
}

int Lpf2Hub::getFirmwareVersionBugfix()
{
    return _lpf2HubFirmwareVersionBugfix;
}

int Lpf2Hub::getFirmwareVersionMajor()
{
    return _lpf2HubFirmwareVersionMajor;
}

int Lpf2Hub::getFirmwareVersionMinor()
{
    return _lpf2HubFirmwareVersionMinor;
}

int Lpf2Hub::getHardwareVersionBuild()
{
    return _lpf2HubHardwareVersionBuild;
}

int Lpf2Hub::getHardwareVersionBugfix()
{
    return _lpf2HubHardwareVersionBugfix;
}

int Lpf2Hub::getHardwareVersionMajor()
{
    return _lpf2HubHardwareVersionMajor;
}

int Lpf2Hub::getHardwareVersionMinor()
{
    return _lpf2HubHardwareVersionMinor;
}

HubType Lpf2Hub::getHubType()
{
    return _hubType;
}

bool Lpf2Hub::isButtonPressed()
{
    return _lpf2HubHubButtonPressed;
}

bool Lpf2Hub::isLeftRemoteUpButtonPressed()
{
    return _lpf2HubRemoteLeftUpButtonPressed;
}

bool Lpf2Hub::isLeftRemoteDownButtonPressed()
{
    return _lpf2HubRemoteLeftDownButtonPressed;
}

bool Lpf2Hub::isLeftRemoteStopButtonPressed()
{
    return _lpf2HubRemoteLeftStopButtonPressed;
}

bool Lpf2Hub::isLeftRemoteButtonReleased()
{
    return _lpf2HubRemoteLeftButtonReleased;
}

bool Lpf2Hub::isRightRemoteUpButtonPressed()
{
    return _lpf2HubRemoteRightUpButtonPressed;
}

bool Lpf2Hub::isRightRemoteDownButtonPressed()
{
    return _lpf2HubRemoteRightDownButtonPressed;
}

bool Lpf2Hub::isRightRemoteStopButtonPressed()
{
    return _lpf2HubRemoteRightStopButtonPressed;
}

bool Lpf2Hub::isRightRemoteButtonReleased()
{
    return _lpf2HubRemoteRightButtonReleased;
}
