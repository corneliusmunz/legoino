/*
 * Legoino.cpp - Arduino Library for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "Legoino.h"

// BLE properties
HubType _hubType;
BLEUUID _bleUuid;
BLEUUID _charachteristicUuid;
BLEAddress *_pServerAddress;
BLERemoteCharacteristic *_pRemoteCharacteristic;

// status properties
boolean _isConnecting = false;
boolean _isConnected = false;

// device properties
int _rssi = -100;
int _batteryLevel = 100; //%
int _voltage = 0;
int _current = 0;

// Notification callbacks
ButtonCallback _buttonCallback = nullptr;
PortCallback _portCallback = nullptr;

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{

    void onResult(BLEAdvertisedDevice advertisedDevice)
    {

        // Found a device, check if the service is contained
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(_bleUuid))
        {

            advertisedDevice.getScan()->stop();

            _pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            _isConnecting = true;
        }
    }
};

static uint8_t ReadUInt8(uint8_t *data, int offset = 0)
{
    uint8_t value = data[0 + offset];
    return value;
}

static int8_t ReadInt8(uint8_t *data, int offset = 0)
{
    int8_t value = (int8_t)data[0 + offset];
    return value;
}

static uint16_t ReadUInt16LE(uint8_t *data, int offset = 0)
{
    uint16_t value = data[0 + offset] | (uint16_t)(data[1 + offset] << 8);
    return value;
}

static int16_t ReadInt16LE(uint8_t *data, int offset = 0)
{
    int16_t value = data[0 + offset] | (int16_t)(data[1 + offset] << 8);
    return value;
}

static uint32_t ReadUInt32LE(uint8_t *data, int offset = 0)
{
    uint32_t value = data[0 + offset] | (uint32_t)(data[1 + offset] << 8) | (uint32_t)(data[2 + offset] << 16) | (uint32_t)(data[3 + offset] << 24);
    return value;
}

/**
 * @brief Convert a raw port value to the corresponding Port datatype
 * @param [in] rawPortValue The raw port value
 */
Port getPortForRawValue(byte rawPortValue)
{
    if (rawPortValue == 0x00)
    {
        return A;
    }

    if (rawPortValue == 0x01)
    {
        return B;
    }

    if (rawPortValue == 0x39)
    {
        return AB;
    }

    return A;
}

/**
 * @brief Parse the incoming characteristic notification for a Device Info Message
 * @param [in] pData The pointer to the received data
 */
void parseDeviceInfo(uint8_t *pData)
{
    Serial.println("parseDeviceInfo");
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
        Serial.print("device name: ");
        Serial.print(name);
        Serial.println();
    }
    // Button press reports
    else if (pData[3] == 0x02)
    {
        if (pData[5] == 1)
        {
            Serial.println("button PRESSED");
            if (_buttonCallback != nullptr)
            {
                _buttonCallback(true);
            }
            return;
        }
        else if (pData[5] == 0)
        {
            if (_buttonCallback != nullptr)
            {
                _buttonCallback(false);
            }
            Serial.println("button RELEASED");
            return;
        }
        // Firmware version
    }
    else if (pData[3] == 0x03)
    {
        int build = ReadUInt16LE(pData, 5);
        int bugfix = ReadUInt8(pData, 7);
        int major = ReadUInt8(pData, 8) >> 4;
        int minor = ReadUInt8(pData, 8) & 0xf;

        Serial.print("Firmware version major:");
        Serial.print(major);
        Serial.print(" minor:");
        Serial.print(minor);
        Serial.print(" bugfix:");
        Serial.print(bugfix);
        Serial.print(" build:");
        Serial.print(build);
        Serial.println();
        // Hardware version
    }
    else if (pData[3] == 0x04)
    {
        int build = ReadUInt16LE(pData, 5);
        int bugfix = ReadUInt8(pData, 7);
        int major = ReadUInt8(pData, 8) >> 4;
        int minor = ReadUInt8(pData, 8) & 0xf;

        Serial.print("Hardware version major:");
        Serial.print(major);
        Serial.print(" minor:");
        Serial.print(minor);
        Serial.print(" bugfix:");
        Serial.print(bugfix);
        Serial.print(" build:");
        Serial.print(build);
        Serial.println();
        // RSSI
    }
    else if (pData[3] == 0x05)
    {
        Serial.print("RSSI update: ");
        _rssi = ReadInt8(pData, 5);
        Serial.print(_rssi);
        Serial.println();
        // Battery level reports
    }
    else if (pData[3] == 0x06)
    {
        _batteryLevel = ReadUInt8(pData, 5);
        Serial.print("Battery level: ");
        Serial.print(_batteryLevel);
        Serial.print("%");
        Serial.println();
        // Battery type
    }
    else if (pData[3] == 0x07)
    {
        Serial.print("Battery type: ");
        if (pData[5] == 0x00)
        {
            Serial.print("Normal");
        }
        else if (pData[5] == 0x01)
        {
            Serial.print("Recharchable");
        }
        Serial.println();
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Port Message
 * @param [in] pData The pointer to the received data
 */
void parsePortMessage(uint8_t *pData)
{
    Serial.println("parsePortMessage");
    byte port = pData[3];
    bool isConnected = (pData[4] == 1 || pData[4] == 2) ? true : false;
    Serial.print("Port ");
    Serial.print(port, HEX);
    if (isConnected)
    {
        Serial.print(" is connected");
    }
    else
    {
        Serial.print(" is not connected");
    }
    if (_portCallback != nullptr)
    {
        _portCallback(getPortForRawValue(port), isConnected);
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Sensor Message
 * @param [in] pData The pointer to the received data
 */
void parseSensorMessage(uint8_t *pData)
{
    Serial.println("parseSensorMessage");
    if (pData[3] == 0x3b)
    {
        int current = ReadUInt16LE(pData, 4);
        Serial.print("Current of Sensor value: ");
        Serial.print(current);
        Serial.println();
        return;
    }
    else if (pData[3] == 0x3c)
    {
        int voltage = ReadUInt16LE(pData, 4);
        Serial.print("Voltage of Sensor value: ");
        Serial.print(voltage);
        Serial.println();
        return;
    }

    switch (pData[4])
    {
    case 0x01:
    {
        Serial.println("Button: UP");

        break;
    }
    case 0xff:
    {
        Serial.println("Button: DOWN");
        break;
    }
    case 0x7f:
    {
        Serial.println("Button: STOP");
        break;
    }
    case 0x00:
    {
        Serial.println("Button: RELEASED");
        break;
    }
    }
}

/**
 * @brief Parse the incoming characteristic notification for a Port Action Message
 * @param [in] pData The pointer to the received data
 */
void parsePortAction(uint8_t *pData)
{
    Serial.println("parsePortAction");
}

/**
 * @brief Callback function for notifications of a specific characteristic
 * @param [in] pBLERemoteCharacteristic The pointer to the characteristic
 * @param [in] pData The pointer to the received data
 * @param [in] length The length of the data array
 * @param [in] isNotify 
 */
void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print("data: ");

    for (int i = 0; i < length; i++)
    {
        Serial.print(pData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

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

Legoino::Legoino(){};

/**
 * @brief Init function to define the Hub type and set the UUIDs 
 * @param [in] hubType (WEDO, BOOST, POWEREDUP)
 */
void Legoino::init(HubType hubType)
{
    _hubType = hubType;
    if (hubType == WEDO2_SMART_HUB)
    {
        _bleUuid = BLEUUID(WEDO_UUID);
        _charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
    }
    else
    {
        _bleUuid = BLEUUID(LPF2_UUID);
        _charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
    };

    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
}

/**
 * @brief Register the callback function if a button message is received
 * @param [in] buttonCallback Function pointer to the callback function which handles the button notification
 */
void Legoino::registerButtonCallback(ButtonCallback buttonCallback)
{
    _buttonCallback = buttonCallback;
}

/**
 * @brief Register the callback function if a port message is received
 * @param [in] portCallback Function pointer to the callback function which handles the port notification
 */
void Legoino::registerPortCallback(PortCallback portCallback)
{
    _portCallback = portCallback;
}

/**
 * @brief Set the color of the HUB LED with predefined colors
 * @param [in] color one of the available hub colors
 */
void Legoino::setLedColor(Color color)
{
    byte setColorMode[10] = {0xA, 0x00, 0x41, 0x32, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    _pRemoteCharacteristic->writeValue(setColorMode, sizeof(setColorMode));
    byte setColor[8] = {0x08, 0x00, 0x81, 0x32, 0x11, 0x51, 0x00, color};
    _pRemoteCharacteristic->writeValue(setColor, sizeof(setColor));
}

/**
 * @brief Set the color of the HUB LED with RGB values 
 * @param [in] red 0..255 
 * @param [in] green 0..255 
 * @param [in] blue 0..255 
 */
void Legoino::setLedRGBColor(char red, char green, char blue)
{
    byte setRGBMode[10] = {0xA, 0x00, 0x41, 0x32, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00};
    _pRemoteCharacteristic->writeValue(setRGBMode, sizeof(setRGBMode));
    byte setRGBColor[10] = {0xA, 0x00, 0x81, 0x32, 0x11, 0x51, 0x01, red, green, blue};
    _pRemoteCharacteristic->writeValue(setRGBColor, sizeof(setRGBColor));
}

/**
 * @brief Send the Shutdown command to the HUB
 */
void Legoino::shutDownHub()
{
    byte shutdownCommand[4] = {0x04, 0x00, 0x02, 0X01};
    _pRemoteCharacteristic->writeValue(shutdownCommand, sizeof(shutdownCommand), true);
}

/**
 * @brief Set name of the HUB
 * @param [in] name character array which contains the name (max 14 characters are supported)
 */
void Legoino::setHubName(char name[])
{

    int nameLength = strlen(name);
    if (nameLength > 14)
    {
        return;
    }

    char offset = 5;
    int arraySize = offset + nameLength;
    byte setName[arraySize] = {arraySize, 0x00, 0x01, 0x01, 0x01};

    for (int idx = 0; idx < nameLength; idx++)
    {
        setName[idx + offset] = name[idx];
    }

    _pRemoteCharacteristic->writeValue(setName, sizeof(setName), true);
}


/**
 * @brief Set the motor speed on a defined port. 
 * @param [in] port Port of the Hub on which the speed of the motor will set (A, B, AB)
 * @param [in] speed Speed of the Motor -100..0..100 negative values will reverse the rotation
 */
void Legoino::setMotorSpeed(Port port, int speed)
{

    byte rawSpeed;
    if (speed == 0)
    {
        rawSpeed = 127; // stop motor
    }
    else if (speed > 0)
    {
        rawSpeed = map(speed, 0, 100, 0, 126);
    }
    else
    {
        rawSpeed = map(-speed, 0, 100, 255, 128);
    }
    byte setMotorCommand[10] = {0xA, 0x00, 0x81, port, 0x11, 0x60, 0x00, rawSpeed, 0x00, 0x00};
    _pRemoteCharacteristic->writeValue(setMotorCommand, sizeof(setMotorCommand), false);
}

/**
 * @brief Stop the motor on a defined port. If no port is set, all motors (AB) will be stopped
 * @param [in] port Port of the Hub on which the motor will be stopped (A, B, AB)
 */
void Legoino::stopMotor(Port port = AB)
{
    setMotorSpeed(port, 0);
}

void activateHubUpdates()
{
    // Activate reports
    byte setButtonCommand[5] = {0x05, 0x00, 0x01, 0x02, 0x02};
    _pRemoteCharacteristic->writeValue(setButtonCommand, sizeof(setButtonCommand));
    byte setBatteryLevelCommand[5] = {0x05, 0x00, 0x01, 0x06, 0x02};
    _pRemoteCharacteristic->writeValue(setBatteryLevelCommand, sizeof(setBatteryLevelCommand));
    byte setRSSICommand[5] = {0x05, 0x00, 0x01, 0x05, 0x02};
    _pRemoteCharacteristic->writeValue(setRSSICommand, sizeof(setRSSICommand));
    //byte setCurrentReport[10] = {0xA, 0x00, 0x41, 0x3b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01};
    //_pRemoteCharacteristic->writeValue(setCurrentReport, sizeof(setCurrentReport), true);
    byte setNameCommand[5] = {0x05, 0x00, 0x01, 0x01, 0x02};
    _pRemoteCharacteristic->writeValue(setNameCommand, sizeof(setNameCommand));
    byte setFWCommand[5] = {0x05, 0x00, 0x01, 0x03, 0x02};
    _pRemoteCharacteristic->writeValue(setFWCommand, sizeof(setFWCommand));
    byte setHWCommand[5] = {0x05, 0x00, 0x01, 0x04, 0x02};
    _pRemoteCharacteristic->writeValue(setHWCommand, sizeof(setHWCommand));
    byte setBatteryType[5] = {0x05, 0x00, 0x01, 0x07, 0x02};
    _pRemoteCharacteristic->writeValue(setBatteryType, sizeof(setBatteryType));
}

/**
 * @brief Connect to the HUB, get a reference to the characteristic and register for notifications
 */
bool Legoino::connectHub()
{
    BLEAddress pAddress = *_pServerAddress;
    BLEClient *pClient = BLEDevice::createClient();

    // Connect to the remove BLE Server (HUB)
    pClient->connect(pAddress);

    BLERemoteService *pRemoteService = pClient->getService(_bleUuid);
    if (pRemoteService == nullptr)
    {
        return false;
    }
    _pRemoteCharacteristic = pRemoteService->getCharacteristic(_charachteristicUuid);
    if (_pRemoteCharacteristic == nullptr)
    {
        return false;
    }

    // register notifications (callback function) for the characteristic
    if (_pRemoteCharacteristic->canNotify())
    {
        _pRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    activateHubUpdates();

    // Set states
    _isConnected = true;
    _isConnecting = false;
    return true;
}

/**
 * @brief Retrieve the connection state. The BLE client (ESP32) has found a service with the desired UUID (HUB)
 * If this state is available, you can try to connect to the Hub
 */
bool Legoino::isConnecting()
{
    return _isConnecting;
}

/**
 * @brief Retrieve the connection state. The BLE client (ESP32) is connected to the server (HUB)
 */
bool Legoino::isConnected()
{
    return _isConnected;
}