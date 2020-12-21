/*
 * Lpf2Hub.cpp - Arduino base class for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#if defined(ESP32)

#include "Lpf2Hub.h"

/** 
 * Callback if a scan has ended with the results of found devices 
 * only needed to enforce the non blocking scan start
 */
void scanEndedCallback(NimBLEScanResults results)
{
    log_d("Number of devices: %d", results.getCount());
    for (int i = 0; i < results.getCount(); i++)
    {
        log_d("device[%d]: %s", i, results.getDevice(i).toString().c_str());
    }
}

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
        log_d("disconnected client");
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
        log_d("advertised device: %s", advertisedDevice->toString().c_str());

        if (advertisedDevice->haveServiceUUID() && advertisedDevice->getServiceUUID().equals(_lpf2Hub->_bleUuid) && (_lpf2Hub->_requestedDeviceAddress == nullptr || (_lpf2Hub->_requestedDeviceAddress && advertisedDevice->getAddress().equals(*_lpf2Hub->_requestedDeviceAddress))))
        {
            advertisedDevice->getScan()->stop();
            _lpf2Hub->_pServerAddress = new BLEAddress(advertisedDevice->getAddress());
            _lpf2Hub->_hubName = advertisedDevice->getName();

            if (advertisedDevice->haveManufacturerData())
            {
                uint8_t *manufacturerData = (uint8_t *)advertisedDevice->getManufacturerData().data();
                uint8_t manufacturerDataLength = advertisedDevice->getManufacturerData().length();
                if (manufacturerDataLength >= 3)
                {
                    log_d("manufacturer data hub type: %x", manufacturerData[3]);
                    //check device type ID
                    switch (manufacturerData[3])
                    {
                    case DUPLO_TRAIN_HUB_ID:
                        _lpf2Hub->_hubType = HubType::DUPLO_TRAIN_HUB;
                        break;
                    case BOOST_MOVE_HUB_ID:
                        _lpf2Hub->_hubType = HubType::BOOST_MOVE_HUB;
                        break;
                    case POWERED_UP_HUB_ID:
                        _lpf2Hub->_hubType = HubType::POWERED_UP_HUB;
                        break;
                    case POWERED_UP_REMOTE_ID:
                        _lpf2Hub->_hubType = HubType::POWERED_UP_REMOTE;
                        break;
                    case CONTROL_PLUS_HUB_ID:
                        _lpf2Hub->_hubType = HubType::CONTROL_PLUS_HUB;
                        break;
                    case MARIO_HUB_ID:
                        _lpf2Hub->_hubType = HubType::MARIO_HUB;
                        break;
                    default:
                        _lpf2Hub->_hubType = HubType::UNKNOWNHUB;
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
    byte commandWithCommonHeader[size + 2] = {(byte)(size + 2), 0x00};
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
    log_d("port: %x, device type: %x", portNumber, deviceType);
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
    log_d("port: %x", portNumber);

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
    if (numberOfConnectedDevices > 0)
    {
        numberOfConnectedDevices--;
    }
}

/**
 * @brief Activate device for receiving updates. E.g. activate a color/distance sensor to
 * write updates on the characteristic if a value has changed. An optional callback could be
 * regeistered here. This function will be called if the update event will occur. 
 * 
 * @param [in] port number where the device is connected
 * @param [in] callback function which will be called on an update event
 */
void Lpf2Hub::activatePortDevice(byte portNumber, PortValueChangeCallback portValueChangeCallback)
{
    byte deviceType = getDeviceTypeForPortNumber(portNumber);
    activatePortDevice(portNumber, deviceType, portValueChangeCallback);
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
void Lpf2Hub::activatePortDevice(byte portNumber, byte deviceType, PortValueChangeCallback portValueChangeCallback)
{
    byte mode = getModeForDeviceType(deviceType);
    log_d("port: %x, device type: %x, callback: %x, mode: %x", portNumber, deviceType, portValueChangeCallback, mode);
    int deviceIndex = getDeviceIndexForPortNumber(portNumber);
    if (deviceIndex < 0)
    {
        return;
    }
    connectedDevices[deviceIndex].Callback = portValueChangeCallback;
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
 * @brief Parse the incoming characteristic notification for a Device Info Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parseDeviceInfo(uint8_t *pData)
{
    if (_hubPropertyChangeCallback != nullptr)
    {
        _hubPropertyChangeCallback(this, (HubPropertyReference)pData[3], pData);
        return;
    }

    if (pData[3] == (byte)HubPropertyReference::ADVERTISING_NAME)
    {
        log_d("advertising name: %s", parseHubAdvertisingName(pData).c_str());
        return;
    }
    else if (pData[3] == (byte)HubPropertyReference::BUTTON)
    {
        log_d("hub button state: %x", parseHubButton(pData));
        return;
    }
    else if (pData[3] == (byte)HubPropertyReference::FW_VERSION)
    {
        Version version = parseVersion(pData);
        log_d("version: %d-%d-%d (%d)", version.Major, version.Minor, version.Bugfix, version.Build);
        return;
    }
    else if (pData[3] == (byte)HubPropertyReference::HW_VERSION)
    {
        Version version = parseVersion(pData);
        log_d("version: %d-%d-%d (%d)", version.Major, version.Minor, version.Bugfix, version.Build);
        return;
    }
    else if (pData[3] == (byte)HubPropertyReference::RSSI)
    {
        log_d("rssi: ", parseRssi(pData));
        return;
    }
    else if (pData[3] == (byte)HubPropertyReference::BATTERY_VOLTAGE)
    {
        log_d("battery level: %d %", parseBatteryLevel(pData));
        return;
    }
    else if (pData[3] == (byte)HubPropertyReference::BATTERY_TYPE)
    {
        log_d("battery type: %d", parseBatteryType(pData));
        return;
    }
    else if (pData[3] == (byte)HubPropertyReference::SYSTEM_TYPE_ID)
    {
        log_d("system type id: %x", parseSystemTypeId(pData));
        return;
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Port Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parsePortMessage(uint8_t *pData)
{
    byte port = pData[3];
    bool isConnected = (pData[4] == 1 || pData[4] == 2) ? true : false;
    if (isConnected)
    {
        log_d("port %x is connected with device %x", port, pData[5]);
        registerPortDevice(port, pData[5]);
    }
    else
    {
        log_d("port %x is disconnected", port);
        deregisterPortDevice(port);
    }
}

/**
 * @brief Parse Mario pant sensor 
 * @param [in] pData The pointer to the received data
 * @return Pant type
 */
MarioPant Lpf2Hub::parseMarioPant(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt8(pData, 4);
    log_d("Mario Pant: %d", value);
    return (MarioPant)value;
}

/**
 * @brief Parse Mario gesture sensor 
 * @param [in] pData The pointer to the received data
 * @return Gesture
 */
MarioGesture Lpf2Hub::parseMarioGesture(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt16LE(pData, 4);
    log_d("Mario Gesture: %d", value);
    return (MarioGesture)value;
}

/**
 * @brief Parse Mario barcode  sensor 
 * @param [in] pData The pointer to the received data
 * @return MarioBarcode
 */
MarioBarcode Lpf2Hub::parseMarioBarcode(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt16LE(pData, 4);
    log_d("Mario Barcode: %d", value);
    return MarioBarcode(value);
}

/**
 * @brief Parse Mario color sensor 
 * @param [in] pData The pointer to the received data
 * @return MarioColor
 */
MarioColor Lpf2Hub::parseMarioColor(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt16LE(pData, 6);
    log_d("Mario Color: %d", value);
    return (MarioColor)value;
}

/**
 * @brief Parse boost hub tilt sensor message (x axis)
 * @param [in] pData The pointer to the received data
 * @return Degrees of rotation/tilt around the x axis
 */
int Lpf2Hub::parseBoostTiltSensorX(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt8(pData, 4);
    log_d("tilt x: %d", value);
    return value;
}

/**
 * @brief Parse boost hub tilt sensor message (y axis)
 * @param [in] pData The pointer to the received data
 * @return Degrees of rotation/tilt around the y axis
 */
int Lpf2Hub::parseBoostTiltSensorY(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt8(pData, 5);
    log_d("tilt y: %d", value);
    return value;
}

/**
 * @brief Parse control plus hub tilt sensor message (x axis)
 * @param [in] pData The pointer to the received data
 * @return Degrees of rotation/tilt around the x axis
 */
int Lpf2Hub::parseControlPlusHubTiltSensorX(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt16LE(pData, 4);
    log_d("tilt x: %d", value);
    return value;
}

/**
 * @brief Parse control plus hub tilt sensor message (y axis)
 * @param [in] pData The pointer to the received data
 * @return Degrees of rotation/tilt around the y axis
 */
int Lpf2Hub::parseControlPlusHubTiltSensorY(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt16LE(pData, 6);
    log_d("tilt y: %d", value);
    return value;
}

/**
 * @brief Parse control plus hub tilt sensor message (z axis)
 * @param [in] pData The pointer to the received data
 * @return Degrees of rotation/tilt around the z axis
 */
int Lpf2Hub::parseControlPlusHubTiltSensorZ(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt16LE(pData, 8);
    log_d("tilt z: %d", value);
    return value;
}

/**
 * @brief Parse current value [mA] of a current sensor message
 * @param [in] pData The pointer to the received data
 * @return current value in unit mA
 */
double Lpf2Hub::parseCurrentSensor(uint8_t *pData)
{
    int currentRaw = LegoinoCommon::ReadUInt16LE(pData, 4);
    double current = (double)currentRaw * LPF2_CURRENT_MAX / LPF2_CURRENT_MAX_RAW;
    log_d("current value: %.2f [mA]", current);
    return current;
}

/**
 * @brief Parse Voltage value [V] of a voltage sensor message
 * @param [in] pData The pointer to the received data
 * @return voltage in unit volt
 */
double Lpf2Hub::parseVoltageSensor(uint8_t *pData)
{
    int voltageRaw = LegoinoCommon::ReadUInt16LE(pData, 4);
    double voltage = (double)voltageRaw * LPF2_VOLTAGE_MAX / LPF2_VOLTAGE_MAX_RAW;
    log_d("voltage value: %.2f [V]", voltage);
    return voltage;
}

/**
 * @brief Parse rotation value [degrees] of a tacho motor
 * @param [in] pData The pointer to the received data
 * @return rotaton in unit degrees (+/-)
 */
int Lpf2Hub::parseTachoMotor(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt32LE(pData, 4);
    log_d("rotation value: %d [degrees]", value);
    return value;
}

/**
 * @brief Parse speed value of a duplo train hub
 * @param [in] pData The pointer to the received data
 * @return speed value
 */
int Lpf2Hub::parseSpeedometer(uint8_t *pData)
{
    int value = LegoinoCommon::ReadInt16LE(pData, 4);
    log_d("speedometer value: %d ", value);
    return value;
}

/**
 * @brief Parse distance value [millimeters] of a distance sensor
 * @param [in] pData The pointer to the received data
 * @return distance in unit millimeters
 */
double Lpf2Hub::parseDistance(uint8_t *pData)
{
    int partial = pData[7];
    double distance = (double)pData[5];
    if (partial > 0)
    {
        distance += 1.0 / partial;
    }
    distance = floor(distance * 25.4) - 20.0;
    log_d("distance : %d", distance);
    return distance;
}

/**
 * @brief Parse detected color value of a color sensor
 * @param [in] pData The pointer to the received data
 * @return detected color (Encoded in Color enum of lego definition)
 */
int Lpf2Hub::parseColor(uint8_t *pData)
{
    int color = pData[4];
    // fix mapping of sensor color data to lego color data
    // this is only needed for green and purple
    if (pData[4] == 1 || pData[4] == 5)
    {
        color = color + 1;
    }
    log_d("color: %s (%d)", LegoinoCommon::ColorStringFromColor(color).c_str(), color);
    return color;
}

/**
 * @brief Parse detected reflectivity value of a duplo train hub color sensor
 * @param [in] pData The pointer to the received data
 * @return reflectivity [%]
 */
int Lpf2Hub::parseReflectivity(uint8_t *pData)
{
    int reflectivity = pData[4];
    log_d("reflectivity: %d [%]", reflectivity);
    return reflectivity;
}

/**
 * @brief Parse button state value of a button sensor
 * @param [in] pData The pointer to the received data
 * @return button state
 */
ButtonState Lpf2Hub::parseRemoteButton(uint8_t *pData)
{
    int buttonState = pData[4];
    log_d("remote button state: %x", buttonState);
    return (ButtonState)buttonState;
}

/**
 * @brief Parse advertising name of hub
 * @param [in] pData The pointer to the received data
 * @return advertising name
 */
std::string Lpf2Hub::parseHubAdvertisingName(uint8_t *pData)
{
    int charArrayLength = min(pData[0] - 5, 14);
    char name[charArrayLength + 1];
    for (int i = 0; i < charArrayLength; i++)
    {
        name[i] = pData[5 + i];
    }
    name[charArrayLength + 1] = 0;
    log_d("advertising name: %s", name);
    return std::string(name);
}

/**
 * @brief Parse button state of hub button
 * @param [in] pData The pointer to the received data
 * @return button state
 */
ButtonState Lpf2Hub::parseHubButton(uint8_t *pData)
{
    int buttonState = pData[5];
    log_d("hub button state: %x", buttonState);
    return (ButtonState)buttonState;
}

/**
 * @brief Parse sw or hw version
 * @param [in] pData The pointer to the received data
 * @return version structure (Major-Minor-Bugfix, Build)
 */
Version Lpf2Hub::parseVersion(uint8_t *pData)
{
    Version version;
    version.Build = LegoinoCommon::ReadUInt16LE(pData, 5);
    version.Major = LegoinoCommon::ReadUInt8(pData, 8) >> 4;
    version.Minor = LegoinoCommon::ReadUInt8(pData, 8) & 0xf;
    version.Bugfix = LegoinoCommon::ReadUInt8(pData, 7);

    return version;
}

/**
 * @brief Parse RSSI value
 * @param [in] pData The pointer to the received data
 * @return RSSI value in dB
 */
int Lpf2Hub::parseRssi(uint8_t *pData)
{
    int rssi = LegoinoCommon::ReadInt8(pData, 5);
    log_d("rssi: %d", rssi);
    return rssi;
}

/**
 * @brief Parse battery level of hub
 * @param [in] pData The pointer to the received data
 * @return battery level in [%]
 */
uint8_t Lpf2Hub::parseBatteryLevel(uint8_t *pData)
{
    uint8_t batteryLevel = LegoinoCommon::ReadUInt8(pData, 5);
    log_d("battery level: %d", batteryLevel);
    return batteryLevel;
}

/**
 * @brief Parse battery type of hub
 * @param [in] pData The pointer to the received data
 * @return battery type (0=normal, 1=recharchable)
 */
byte Lpf2Hub::parseBatteryType(uint8_t *pData)
{
    byte batteryType = pData[5];
    log_d("battery type: %x", batteryType);
    return batteryType;
}

/**
 * @brief Parse system type id of hub
 * @param [in] pData The pointer to the received data
 * @return system type id
 */
uint8_t Lpf2Hub::parseSystemTypeId(uint8_t *pData)
{
    uint8_t systemTypeId = LegoinoCommon::ReadUInt8(pData, 5);
    return systemTypeId;
}

/**
 * @brief Get the update mode dependent on the device type
 * @param [in] pData The pointer to the received data
 * @return Update mode
 */
byte Lpf2Hub::getModeForDeviceType(byte deviceType)
{
    switch (deviceType)
    {
    case (byte)DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::TRAIN_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::MEDIUM_LINEAR_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::MOVE_HUB_MEDIUM_LINEAR_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::COLOR_DISTANCE_SENSOR:
        return 0x08;
    case (byte)DeviceType::MOVE_HUB_TILT_SENSOR:
        return 0x00;
    case (byte)DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::TECHNIC_LARGE_LINEAR_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::TECHNIC_XLARGE_LINEAR_MOTOR:
        return (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM;
    case (byte)DeviceType::MARIO_HUB_GESTURE_SENSOR:
        return 0x01;
    default:
        return 0x00;
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Sensor Message if a callback 
 * is registered, the callback of that connected device is called with the received data
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parseSensorMessage(uint8_t *pData)
{
    int deviceIndex = getDeviceIndexForPortNumber(pData[3]);
    if (deviceIndex < 0)
    {
        return;
    }

    byte deviceType = connectedDevices[deviceIndex].DeviceType;

    if (connectedDevices[deviceIndex].Callback != nullptr)
    {
        connectedDevices[deviceIndex].Callback(this, pData[3], (DeviceType)deviceType, pData);
        return;
    }

    if (deviceType == (byte)DeviceType::CURRENT_SENSOR)
    {
        parseCurrentSensor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::VOLTAGE_SENSOR)
    {
        parseVoltageSensor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::MEDIUM_LINEAR_MOTOR || deviceType == (byte)DeviceType::MOVE_HUB_MEDIUM_LINEAR_MOTOR)
    {
        parseTachoMotor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::DUPLO_TRAIN_BASE_SPEEDOMETER)
    {
        parseSpeedometer(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::DUPLO_TRAIN_BASE_COLOR_SENSOR)
    {
        parseColor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::COLOR_DISTANCE_SENSOR)
    {
        parseDistance(pData);
        parseColor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::MOVE_HUB_TILT_SENSOR)
    {
        parseBoostTiltSensorX(pData);
        parseBoostTiltSensorY(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::TECHNIC_MEDIUM_HUB_TILT_SENSOR)
    {
        parseControlPlusHubTiltSensorX(pData);
        parseControlPlusHubTiltSensorY(pData);
        parseControlPlusHubTiltSensorZ(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::REMOTE_CONTROL_BUTTON)
    {
        int port = pData[3];
        parseRemoteButton(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::MARIO_HUB_GESTURE_SENSOR)
    {
        int port = pData[3];
        parseMarioGesture(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::MARIO_HUB_BARCODE_SENSOR)
    {
        int port = pData[3];
        parseMarioBarcode(pData);
        parseMarioColor(pData);
        return;
    }
    else if (deviceType == (byte)DeviceType::MARIO_HUB_PANT_SENSOR)
    {
        int port = pData[3];
        parseMarioPant(pData);
        return;
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Port Action Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parsePortAction(uint8_t *pData)
{
    log_d("parsePortAction");
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
    log_d("notify callback for characteristic %s", pBLERemoteCharacteristic->getUUID().toString().c_str());

    switch (pData[2])
    {
    case (byte)MessageType::HUB_PROPERTIES:
    {
        parseDeviceInfo(pData);
        break;
    }
    case (byte)MessageType::HUB_ATTACHED_IO:
    {
        parsePortMessage(pData);
        break;
    }
    case (byte)MessageType::PORT_VALUE_SINGLE:
    {
        parseSensorMessage(pData);
        break;
    }
    case (byte)MessageType::PORT_OUTPUT_COMMAND_FEEDBACK:
    {
        parsePortAction(pData);
        break;
    }
    }
}

/**
 * @brief Constructor
 */
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
    // start method with callback function to enforce the non blocking scan. If no callback function is used,
    // the scan starts in a blocking manner
    pBLEScan->start(_scanDuration, scanEndedCallback);
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

/**
 * @brief Init function set the BLE scan duration (default value 5s)
 * @param [in] BLE scan durtation in unit seconds
 */
void Lpf2Hub::init(uint32_t scanDuration)
{
    _scanDuration = scanDuration;
    init();
}

/**
 * @brief Init function set the BLE scan duration (default value 5s)
 * @param [in] deviceAddress to which the arduino should connect represented by a hex string of the format: 00:00:00:00:00:00
 * @param [in] BLE scan durtation in unit seconds
 */
void Lpf2Hub::init(std::string deviceAddress, uint32_t scanDuration)
{
    _requestedDeviceAddress = new BLEAddress(deviceAddress);
    _scanDuration = scanDuration;
    init();
}

/**
 * @brief Get the address of the HUB (server address)
 * @return HUB Address
 */
NimBLEAddress Lpf2Hub::getHubAddress()
{
    NimBLEAddress pAddress = *_pServerAddress;
    return pAddress;
}

/**
 * @brief Get the array index of a specific connected device on a defined port in the connectedDevices array
 * @param [in] port number
 * @return array index of the connected device
 */
int Lpf2Hub::getDeviceIndexForPortNumber(byte portNumber)
{
    log_d("Number of connected devices: %d", numberOfConnectedDevices);
    for (int idx = 0; idx < numberOfConnectedDevices; idx++)
    {
        log_v("device %d, port number: %x, device type: %x, callback address: %x", idx, connectedDevices[idx].PortNumber, connectedDevices[idx].DeviceType, connectedDevices[idx].Callback);
        if (connectedDevices[idx].PortNumber == portNumber)
        {
            log_d("device on port %x has index %d", portNumber, idx);
            return idx;
        }
    }
    log_w("no device found for port number %x", portNumber);
    return -1;
}

/**
 * @brief Get the device type of a specific connected device on a defined port in the connectedDevices array
 * @param [in] port number
 * @return device type of the connected device
 */
byte Lpf2Hub::getDeviceTypeForPortNumber(byte portNumber)
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

/**
 * @brief Get the port where a specific device is connected
 * @param [in] device type
 * @return port number if device type is found or 255
 */
byte Lpf2Hub::getPortForDeviceType(byte deviceType)
{
    log_d("Number of connected devices: %d", numberOfConnectedDevices);
    for (int idx = 0; idx < numberOfConnectedDevices; idx++)
    {
        log_v("device %d, port number: %x, device type: %x, callback address: %x", idx, connectedDevices[idx].PortNumber, connectedDevices[idx].DeviceType, connectedDevices[idx].Callback);
        if (connectedDevices[idx].DeviceType == deviceType)
        {
            log_d("port %x has device of type %x", connectedDevices[idx].PortNumber, deviceType);
            return connectedDevices[idx].PortNumber;
        }
    }
    log_w("no port found with device type %x", deviceType);
    return 255;
}

/**
 * @brief Set the color of the HUB LED with predefined colors
 * @param [in] color one of the available hub colors
 */
void Lpf2Hub::setLedColor(Color color)
{
    byte port = getPortForDeviceType((byte)DeviceType::HUB_LED);
    byte setColorMode[8] = {0x41, port, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(setColorMode, 8);
    byte setColor[6] = {0x81, port, 0x11, 0x51, 0x00, color};
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
    byte port = getPortForDeviceType((byte)DeviceType::HUB_LED);
    byte setRGBMode[8] = {0x41, port, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(setRGBMode, 8);
    byte setRGBColor[8] = {0x81, port, 0x11, 0x51, 0x01, red, green, blue};
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
    _hubName = std::string(name, nameLength);

    char offset = 3;
    int arraySize = offset + nameLength;
    byte setNameCommand[arraySize] = {0x01, 0x01, 0x01};

    memcpy(setNameCommand + offset, name, nameLength);
    WriteValue(setNameCommand, arraySize);
}

/**
 * @brief Activate the update/notification of hub specific property changes (battery level, rssi, ...)
 * @param [in] hubProperty for which updates should be activated
 * @param [in] optional callback function which will be called if a value has changed
 */
void Lpf2Hub::activateHubPropertyUpdate(HubPropertyReference hubProperty, HubPropertyChangeCallback hubPropertyChangeCallback)
{
    if (hubPropertyChangeCallback != nullptr)
    {
        _hubPropertyChangeCallback = hubPropertyChangeCallback;
    }

    // Activate reports
    byte notifyPropertyCommand[3] = {0x01, (byte)hubProperty, (byte)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM};
    WriteValue(notifyPropertyCommand, 3);
}

/**
 * @brief Request a hub specific property value (battery level, rssi, ...)
 * @param [in] hubProperty for which the value should be requested
 * @param [in] optional callback function which will be called if a value has changed
 */
void Lpf2Hub::requestHubPropertyUpdate(HubPropertyReference hubProperty, HubPropertyChangeCallback hubPropertyChangeCallback)
{
    if (hubPropertyChangeCallback != nullptr)
    {
        _hubPropertyChangeCallback = hubPropertyChangeCallback;
    }

    // Activate reports
    byte notifyPropertyCommand[3] = {0x01, (byte)hubProperty, (byte)HubPropertyOperation::REQUEST_UPDATE_DOWNSTREAM};
    WriteValue(notifyPropertyCommand, 3);
}

/**
 * @brief Deactivate the update/notification of hub specific property changes (battery level, rssi, ...)
 * @param [in] hubProperty for which updates should be activated
 */
void Lpf2Hub::deactivateHubPropertyUpdate(HubPropertyReference hubProperty)
{

    // Activate reports
    byte notifyPropertyCommand[3] = {0x01, (byte)hubProperty, (byte)HubPropertyOperation::DISABLE_UPDATES_DOWNSTREAM};
    WriteValue(notifyPropertyCommand, 3);
}

/**
 * @brief Connect to the HUB, get a reference to the characteristic and register for notifications
 */
bool Lpf2Hub::connectHub()
{
    BLEAddress pAddress = *_pServerAddress;
    NimBLEClient *pClient = nullptr;

    log_d("number of ble clients: %d", NimBLEDevice::getClientListSize());

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
                log_e("reconnect failed");
                return false;
            }
            log_d("reconnect client");
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
            log_w("max clients reached - no more connections available: %d", NimBLEDevice::getClientListSize());
            return false;
        }

        pClient = NimBLEDevice::createClient();
    }

    if (!pClient->isConnected())
    {
        if (!pClient->connect(pAddress))
        {
            log_e("failed to connect");
            return false;
        }
    }

    log_d("connected to: %s, RSSI: %d", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
    BLERemoteService *pRemoteService = pClient->getService(_bleUuid);
    if (pRemoteService == nullptr)
    {
        log_e("failed to get ble client");
        return false;
    }

    _pRemoteCharacteristic = pRemoteService->getCharacteristic(_charachteristicUuid);
    if (_pRemoteCharacteristic == nullptr)
    {
        log_e("failed to get ble service");
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

/**
 * @brief Retrieve the hub type
 * @return hub type 
 */
HubType Lpf2Hub::getHubType()
{
    return _hubType;
}

/**
 * @brief Retrieve the hub name
 * @return hub name 
 */
std::string Lpf2Hub::getHubName()
{
    return _hubName;
}

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 */
void Lpf2Hub::setBasicMotorSpeed(byte port, int speed = 0)
{
    byte setMotorCommand[8] = {0x81, port, 0x11, 0x51, 0x00, LegoinoCommon::MapSpeed(speed)}; //train, batmobil
    WriteValue(setMotorCommand, 6);
}

/**
 * @brief Stop the motor on a defined port.
 * @param [in] port Port of the Hub on which the motor will be stopped (A, B)
 */
void Lpf2Hub::stopBasicMotor(byte port)
{
    setBasicMotorSpeed(port, 0);
}

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 */
void Lpf2Hub::setTachoMotorSpeed(byte port, int speed, byte maxPower, BrakingStyle brakingStyle)
{
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[8] = {0x81, port, 0x11, 0x01, LegoinoCommon::MapSpeed(speed), maxPower, (byte)brakingStyle, 0x03};
    WriteValue(setMotorCommand, 8);
}

/**
 * @brief Stop the motor on a defined port.
 * @param [in] port Port of the Hub on which the motor will be stopped (A, B, AB, C, D)
 */
void Lpf2Hub::stopTachoMotor(byte port)
{
    setTachoMotorSpeed(port, 0);
}

/**
 * @brief Set the acceleration profile 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] time Time value in ms of the acceleration from 0-100% speed/Power
 */
void Lpf2Hub::setAccelerationProfile(byte port, int16_t time)
{
    byte *timeBytes = LegoinoCommon::Int16ToByteArray(time);
    byte setMotorCommand[7] = {0x81, port, 0x10, 0x05, timeBytes[0], timeBytes[1], 0x01};
    WriteValue(setMotorCommand, 7);
}

/**
 * @brief Set the deceleration profile 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] time Time value in ms of the deceleration from 100-0% speed/Power
 */
void Lpf2Hub::setDecelerationProfile(byte port, int16_t time)
{
    byte *timeBytes = LegoinoCommon::Int16ToByteArray(time);
    byte setMotorCommand[7] = {0x81, port, 0x10, 0x06, timeBytes[0], timeBytes[1], 0x02};
    WriteValue(setMotorCommand, 7);
}

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 * @param [in] time Time in miliseconds for running the motor on the desired speed
 * @param [in] maximum Power of the Motor 0..100 (default value = 100)
 * @param [in] brakingStyle Braking style how the motor will stop. Brake(default), Float, Hold are available
 */
void Lpf2Hub::setTachoMotorSpeedForTime(byte port, int speed, int16_t time = 0, byte maxPower, BrakingStyle brakingStyle)
{
    //Use acc and dec profile (0x03 last two bits set)
    byte *timeBytes = LegoinoCommon::Int16ToByteArray(time);
    byte setMotorCommand[10] = {0x81, port, 0x11, 0x09, timeBytes[0], timeBytes[1], LegoinoCommon::MapSpeed(speed), maxPower, (byte)brakingStyle, 0x03};
    WriteValue(setMotorCommand, 10);
}

/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 * @param [in] time Time in miliseconds for running the motor on the desired speed
 * @param [in] maximum Power of the Motor 0..100 (default value = 100)
 * @param [in] brakingStyle Braking style how the motor will stop. Brake(default), Float, Hold are available
 */
void Lpf2Hub::setTachoMotorSpeedForDegrees(byte port, int speed, int32_t degrees, byte maxPower, BrakingStyle brakingStyle)
{
    byte *degreeBytes = LegoinoCommon::Int32ToByteArray(degrees);
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[12] = {0x81, port, 0x11, 0x0B, degreeBytes[0], degreeBytes[1], degreeBytes[2], degreeBytes[3], LegoinoCommon::MapSpeed(speed), maxPower, (byte)brakingStyle, 0x03};
    WriteValue(setMotorCommand, 12);
}

/**
 * @brief Set the motor absolute position on a defined port.
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor 0..100 positive values only
 * @param [in] position Position in degrees (relative to zero point on power up, or encoder reset) -2,147,483,648..0..2,147,483,647
 * @param [in] maximum Power of the Motor 0..100 (default value = 100)
 * @param [in] brakingStyle Braking style how the motor will stop. Brake(default), Float, Hold are available
 */
void Lpf2Hub::setAbsoluteMotorPosition(byte port, int speed, int32_t position, byte maxPower, BrakingStyle brakingStyle)
{
    byte *positionBytes = LegoinoCommon::Int32ToByteArray(position);
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[12] = {0x81, port, 0x11, 0x0D, positionBytes[0], positionBytes[1], positionBytes[2], positionBytes[3], LegoinoCommon::MapSpeed(speed), maxPower, (byte)brakingStyle, 0x03};
    WriteValue(setMotorCommand, 12);
}

/**
 * @brief Set the motor encoded position on a defined port.
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] position Position in degrees to encode (0 = Reset) -2,147,483,648..0..2,147,483,647
 */
void Lpf2Hub::setAbsoluteMotorEncoderPosition(byte port, int32_t position)
{
    byte *positionBytes = LegoinoCommon::Int32ToByteArray(position);
    //WriteModeData (0x51)
    //PresetEncoder mode (0x02)
    byte setMotorCommand[9] = {0x81, port, 0x11, 0x51, 0x02, positionBytes[0], positionBytes[1], positionBytes[2], positionBytes[3]};
    WriteValue(setMotorCommand, 9);
}

/**
 * @brief Set the speeds of the MoveHub Motors
 * @param [in] speedLeft Speed of the left motor
 * @param [in] speedRight speed of the right motor
 * @param [in] degrees till which rotation in degrees the motors should run
 * @param [in] maximum Power of the Motor 0..100 (default value = 100)
 * @param [in] brakingStyle Braking style how the motor will stop. Brake(default), Float, Hold are available
 */
void Lpf2Hub::setTachoMotorSpeedsForDegrees(int speedLeft, int speedRight, int32_t degrees, byte maxPower, BrakingStyle brakingStyle)
{
    byte *degreeBytes = LegoinoCommon::Int32ToByteArray(degrees);
    byte port = (byte)MoveHubPort::AB;
    //Use acc and dec profile (0x03 last two bits set)
    byte setMotorCommand[13] = {0x81, port, 0x11, 0x0C, degreeBytes[0], degreeBytes[1], degreeBytes[2], degreeBytes[3], LegoinoCommon::MapSpeed(speedLeft), LegoinoCommon::MapSpeed(speedRight), maxPower, (byte)brakingStyle, 0x03}; //boost with time
    WriteValue(setMotorCommand, 13);
}

/**
 * @brief Play a predefined sound on the Duplo train hub
 * @param [in] sound value. Could be set via the DuploTrainBaseSound enum
 */
void Lpf2Hub::playSound(byte sound)
{
    byte setSoundMode[8] = {0x41, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(setSoundMode, 8);
    byte playSound[6] = {0x81, 0x01, 0x11, 0x51, 0x01, sound};
    WriteValue(playSound, 6);
}

/**
 * @brief Play a predefined tone on the Duplo train hub
 * @param [in] tone value. Different bieps on different number 0..10 Which number is which
 * biep is not really clear right now.
 */
void Lpf2Hub::playTone(byte number)
{
    byte setToneMode[8] = {0x41, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(setToneMode, 8);
    byte playTone[6] = {0x81, 0x01, 0x11, 0x51, 0x02, number};
    WriteValue(playTone, 6);
}

#endif // ESP32