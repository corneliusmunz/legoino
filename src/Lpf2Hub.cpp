/*
 * Lpf2Hub.cpp - Arduino base class for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "Lpf2Hub.h"

Device connectedDevices[10];
int numberOfConnectedDevices = 0;

// Hub orientation
int Lpf2HubTiltX;
int Lpf2HubTiltY;

// Boost tacho motor
int Lpf2HubTachoMotorRotation;

// Distance/Color sensor
double Lpf2HubDistance;
int Lpf2HubColor;

// Hub information values
int Lpf2HubRssi;
uint8_t Lpf2HubBatteryLevel;
int Lpf2HubHubMotorRotation;
bool Lpf2HubHubButtonPressed;

int Lpf2HubFirmwareVersionBuild;
int Lpf2HubFirmwareVersionBugfix;
int Lpf2HubFirmwareVersionMajor;
int Lpf2HubFirmwareVersionMinor;

int Lpf2HubHardwareVersionBuild;
int Lpf2HubHardwareVersionBugfix;
int Lpf2HubHardwareVersionMajor;
int Lpf2HubHardwareVersionMinor;

// PoweredUp Remote
bool Lpf2HubRemoteLeftUpButtonPressed;
bool Lpf2HubRemoteLeftDownButtonPressed;
bool Lpf2HubRemoteLeftStopButtonPressed;
bool Lpf2HubRemoteLeftButtonReleased;

bool Lpf2HubRemoteRightUpButtonPressed;
bool Lpf2HubRemoteRightDownButtonPressed;
bool Lpf2HubRemoteRightStopButtonPressed;
bool Lpf2HubRemoteRightButtonReleased;

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class Lpf2HubAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    Lpf2Hub *_lpf2Hub;

public:
    Lpf2HubAdvertisedDeviceCallbacks(Lpf2Hub *lpf2Hub) : BLEAdvertisedDeviceCallbacks()
    {
        _lpf2Hub = lpf2Hub;
    }

    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        //Found a device, check if the service is contained
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(_lpf2Hub->_bleUuid))
        {
            advertisedDevice.getScan()->stop();
            _lpf2Hub->_pServerAddress = new BLEAddress(advertisedDevice.getAddress());
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
 * @brief Map speed from -100..100 to the 8bit internal value
 * @param [in] speed -100..100
 */
byte Lpf2Hub::MapSpeed(int speed)
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
    return rawSpeed;
}

byte *Lpf2Hub::Int16ToByteArray(int16_t x)
{
    static byte y[2];
    y[0] = (byte)(x & 0xff);
    y[1] = (byte)((x >> 8) & 0xff);
    return y;
}

byte *Lpf2Hub::Int32ToByteArray(int32_t x)
{
    static byte y[4];
    y[0] = (byte)(x & 0xff);
    y[1] = (byte)((x >> 8) & 0xff);
    y[2] = (byte)((x >> 16) & 0xff);
    y[3] = (byte)((x >> 24) & 0xff);
    return y;
}

uint8_t Lpf2Hub::ReadUInt8(uint8_t *data, int offset = 0)
{
    uint8_t value = data[0 + offset];
    return value;
}

int8_t Lpf2Hub::ReadInt8(uint8_t *data, int offset = 0)
{
    int8_t value = (int8_t)data[0 + offset];
    return value;
}

uint16_t Lpf2Hub::ReadUInt16LE(uint8_t *data, int offset = 0)
{
    uint16_t value = data[0 + offset] | (uint16_t)(data[1 + offset] << 8);
    return value;
}

int16_t Lpf2Hub::ReadInt16LE(uint8_t *data, int offset = 0)
{
    int16_t value = data[0 + offset] | (int16_t)(data[1 + offset] << 8);
    return value;
}

uint32_t Lpf2Hub::ReadUInt32LE(uint8_t *data, int offset = 0)
{
    uint32_t value = data[0 + offset] | (uint32_t)(data[1 + offset] << 8) | (uint32_t)(data[2 + offset] << 16) | (uint32_t)(data[3 + offset] << 24);
    return value;
}

int32_t Lpf2Hub::ReadInt32LE(uint8_t *data, int offset = 0)
{
    int32_t value = data[0 + offset] | (int16_t)(data[1 + offset] << 8) | (uint32_t)(data[2 + offset] << 16) | (uint32_t)(data[3 + offset] << 24);
    return value;
}

void Lpf2Hub::activatePortDevice(byte portNumber)
{
    LOGLINE("activatePortDevice(portNumber)");
    byte deviceType = getDeviceTypeForPortNumber(portNumber);
    activatePortDevice(portNumber, deviceType);
}

void Lpf2Hub::activatePortDevice(byte portNumber, byte deviceType)
{
    LOGLINE("activatePortDevice");
    byte mode = getModeForDeviceType(deviceType);
    LOG("mode for device: ");
    LOGLINE(mode, HEX);
    byte activatePortDeviceMessage[8] = {0x41, portNumber, mode, 0x01, 0x00, 0x00, 0x00, 0x01};
    WriteValue(activatePortDeviceMessage, 8);
}

void Lpf2Hub::deactivatePortDevice(byte portNumber)
{
    byte deviceType = getDeviceTypeForPortNumber(portNumber);
    deactivatePortDevice(portNumber, deviceType);
}

void Lpf2Hub::deactivatePortDevice(byte portNumber, byte deviceType)
{
    byte mode = getModeForDeviceType(deviceType);
    byte deactivatePortDeviceMessage[8] = {0x41, portNumber, mode, 0x01, 0x00, 0x00, 0x00, 0x00};
    WriteValue(deactivatePortDeviceMessage, 8);
}

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
            // if (_buttonCallback != nullptr)
            // {
            //     _buttonCallback(true);
            // }
            LOGLINE("button PRESSED");
            Lpf2HubHubButtonPressed = true;
            return;
        }
        else if (pData[5] == 0)
        {
            // if (_buttonCallback != nullptr)
            // {
            //     _buttonCallback(false);
            // }
            LOGLINE("button RELEASED");
            Lpf2HubHubButtonPressed = false;
            return;
        }
    }
    else if (pData[3] == 0x03) // Firmware version
    {
        Lpf2HubFirmwareVersionBuild = ReadUInt16LE(pData, 5);
        Lpf2HubFirmwareVersionBugfix = ReadUInt8(pData, 7);
        Lpf2HubFirmwareVersionMajor = ReadUInt8(pData, 8) >> 4;
        Lpf2HubFirmwareVersionMinor = ReadUInt8(pData, 8) & 0xf;

        LOG("Firmware version major:");
        LOG(Lpf2HubFirmwareVersionMajor);
        LOG(" minor:");
        LOG(Lpf2HubFirmwareVersionMinor);
        LOG(" bugfix:");
        LOG(Lpf2HubFirmwareVersionBugfix);
        LOG(" build:");
        LOG(Lpf2HubFirmwareVersionBuild);
        LOGLINE();
    }
    else if (pData[3] == 0x04) // Hardware version
    {
        Lpf2HubHardwareVersionBuild = ReadUInt16LE(pData, 5);
        Lpf2HubHardwareVersionBugfix = ReadUInt8(pData, 7);
        Lpf2HubHardwareVersionMajor = ReadUInt8(pData, 8) >> 4;
        Lpf2HubHardwareVersionMinor = ReadUInt8(pData, 8) & 0xf;

        LOG("Hardware version major:");
        LOG(Lpf2HubHardwareVersionMajor);
        LOG(" minor:");
        LOG(Lpf2HubHardwareVersionMinor);
        LOG(" bugfix:");
        LOG(Lpf2HubHardwareVersionBugfix);
        LOG(" build:");
        LOG(Lpf2HubHardwareVersionBuild);
        LOGLINE();
    }
    else if (pData[3] == 0x05) // RSSI
    {
        LOG("RSSI update: ");
        Lpf2HubRssi = ReadInt8(pData, 5);
        LOG(Lpf2HubRssi);
        LOGLINE();
    }
    else if (pData[3] == 0x06) // Battery level reports
    {
        Lpf2HubBatteryLevel = ReadUInt8(pData, 5);
        LOG("Battery level: ");
        LOG(Lpf2HubBatteryLevel);
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
        Device newDevice = {port, pData[5]};
        connectedDevices[numberOfConnectedDevices] = newDevice;
        numberOfConnectedDevices++;
    }
    else
    {
        LOGLINE(" is disconnected");
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
}

void Lpf2Hub::parseBoostTiltSensor(uint8_t *pData)
{
    LOGLINE("parseBoostTiltSensor");
    Lpf2HubTiltX = pData[4] > 64 ? map(pData[4], 255, 191, 0, 90) : map(pData[4], 0, 64, 0, -90);
    Lpf2HubTiltY = pData[5] > 64 ? map(pData[5], 255, 191, 0, -90) : map(pData[5], 0, 64, 0, 90);
    LOG("x:");
    LOG(Lpf2HubTiltX, DEC);
    LOG(" y:");
    LOGLINE(Lpf2HubTiltY, DEC);
}

void Lpf2Hub::parseBoostTachoMotor(uint8_t *pData)
{
    LOGLINE("parseBoostTachoMotor");
    Lpf2HubTachoMotorRotation = ReadInt32LE(pData, 4);
    LOG("Tacho motor rotation: ");
    LOGLINE(Lpf2HubTachoMotorRotation, DEC);
}

void Lpf2Hub::parseBoostHubMotor(uint8_t *pData)
{
    LOGLINE("parseBoostHubMotor");
    Lpf2HubHubMotorRotation = ReadInt32LE(pData, 4);
    LOG("BoostHub motor rotation: ");
    LOGLINE(Lpf2HubHubMotorRotation, DEC);
}

void Lpf2Hub::parseBoostDistanceAndColor(uint8_t *pData)
{
    LOGLINE("parseBoostDistanceAndColor");
    int partial = pData[7];
    Lpf2HubColor = pData[4];
    Lpf2HubDistance = (double)pData[5];
    if (partial > 0)
    {
        Lpf2HubDistance += 1.0 / partial;
    }
    Lpf2HubDistance = floor(Lpf2HubDistance * 25.4) - 20.0;

    LOG("Distance: ");
    LOG(Lpf2HubDistance, DEC);
    LOG(" Color: ");
    if (Lpf2HubColor > 10)
    {
        LOGLINE("undefined");
    }
    else
    {
        LOGLINE(COLOR_STRING[Lpf2HubColor]);
    }
}

void Lpf2Hub::parsePoweredUpRemote(uint8_t *pData)
{
    LOGLINE("parsePoweredUp Remote Button");
    int port = pData[3];
    LOG("Port: ");
    LOG(port, DEC);
    int buttonState = pData[4];
    if (buttonState == 0x01)
    {
        LOGLINE(" ButtonState: UP");
        if (port == 0x00) {
            Lpf2HubRemoteLeftUpButtonPressed = true;
        } else if (port == 0x01) {
            Lpf2HubRemoteRightUpButtonPressed = true;
        }
    }
    else if (buttonState == 0xff)
    {
        LOGLINE(" ButtonState: DOWN");
        if (port == 0x00) {
            Lpf2HubRemoteLeftDownButtonPressed = true;
        } else if (port == 0x01) {
            Lpf2HubRemoteRightDownButtonPressed = true;
        }
    }
    else if (buttonState == 0x7f)
    {
        LOGLINE(" ButtonState: STOP");
        if (port == 0x00) {
            Lpf2HubRemoteLeftStopButtonPressed = true;
        } else if (port == 0x01) {
            Lpf2HubRemoteRightStopButtonPressed = true;
        }
    }
    else if (buttonState == 0x00)
    {
        LOGLINE(" ButtonState: RELEASED");
        if (port == 0x00) {
            Lpf2HubRemoteLeftUpButtonPressed = false;
            Lpf2HubRemoteLeftDownButtonPressed = false;
            Lpf2HubRemoteLeftStopButtonPressed = false;
            Lpf2HubRemoteLeftButtonReleased = true;
        } else if (port == 0x01) {
            Lpf2HubRemoteRightUpButtonPressed = false;
            Lpf2HubRemoteRightDownButtonPressed = false;
            Lpf2HubRemoteRightStopButtonPressed = false;
            Lpf2HubRemoteRightButtonReleased = true;            
        }
    }
}

byte Lpf2Hub::getModeForDeviceType(byte deviceType)
{
    switch (deviceType)
    {
    case BASIC_MOTOR:
        return 0x02;
    case BOOST_TACHO_MOTOR:
        return 0x02;
    case BOOST_MOVE_HUB_MOTOR:
        return 0x02;
    case BOOST_DISTANCE:
        return 0x08;
    case BOOST_TILT:
        return 0x04;
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
    byte deviceType = getDeviceTypeForPortNumber(pData[3]);
    if (pData[3] == 0x3b)
    {
        int current = ReadUInt16LE(pData, 4);
        LOG("Current of Sensor value: ");
        LOG(current);
        LOGLINE();
        return;
    }
    else if (pData[3] == 0x3c)
    {
        int voltage = ReadUInt16LE(pData, 4);
        LOG("Voltage of Sensor value: ");
        LOG(voltage);
        LOGLINE();
        return;
    }
    else if (deviceType == BOOST_TACHO_MOTOR)
    {
        parseBoostTachoMotor(pData);
    }
    else if (deviceType == BOOST_DISTANCE)
    {
        parseBoostDistanceAndColor(pData);
    }
    else if (deviceType == BOOST_TILT)
    {
        parseBoostTiltSensor(pData);
    }
    else if (deviceType == POWERED_UP_REMOTE_BUTTON)
    {
        parsePoweredUpRemote(pData);
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
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    //LOG("Notify callback for characteristic ");
    //LOG(pBLERemoteCharacteristic->getUUID().toString().c_str());
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

    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();

    pBLEScan->setAdvertisedDeviceCallbacks(new Lpf2HubAdvertisedDeviceCallbacks(this));

    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
}

/**
 * @brief Register the connected devices to map the ports to the device types
 * @param [in] connectedDevices[] Array to a device struct of all connected devices
 */
void Lpf2Hub::initConnectedDevices(Device devices[], byte deviceNumbers)
{
    numberOfConnectedDevices = deviceNumbers;
    for (int idx = 0; idx < numberOfConnectedDevices; idx++)
    {
        connectedDevices[idx] = devices[idx];
    }
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

    return UNDEFINED;
}

// Device Lpf2Hub::getDeviceForPortNumber(byte portNumber)
// {
//     for (int idx = 0; idx < numberOfConnectedDevices; idx++) {
//         if (connectedDevices[idx].PortNumber == portNumber) {
//             return connectedDevices[idx];
//         }
//     }
//     return UNDEFINED;
// }

// Device Lpf2Hub::getDeviceForDeviceType(byte deviceType)
// {
//     for (int idx = 0; idx < numberOfConnectedDevices; idx++) {
//         if (connectedDevices[idx].DeviceType == deviceType) {
//             return connectedDevices[idx];
//         }
//     }
//     return UNDEFINED;
// }

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

    byte setRSSICommand[3] = {0x01, 0x05, 0x02};
    WriteValue(setRSSICommand, 3);

    //byte setCurrentReport[8] = {0x41, 0x3b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01};
    //WriteValue(setCurrentReport, 8);

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
    BLEClient *pClient = BLEDevice::createClient();

    // Connect to the remove BLE Server (HUB)
    pClient->connect(pAddress);

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
    return Lpf2HubHubMotorRotation;
}

int Lpf2Hub::getRssi()
{
    return Lpf2HubRssi;
}

int Lpf2Hub::getBatteryLevel()
{
    return Lpf2HubBatteryLevel;
}

int Lpf2Hub::getTiltX()
{
    return Lpf2HubTiltX;
}

int Lpf2Hub::getTiltY()
{
    return Lpf2HubTiltY;
}

int Lpf2Hub::getFirmwareVersionBuild()
{
    return Lpf2HubFirmwareVersionBuild;
}

int Lpf2Hub::getFirmwareVersionBugfix()
{
    return Lpf2HubFirmwareVersionBugfix;
}

int Lpf2Hub::getFirmwareVersionMajor()
{
    return Lpf2HubFirmwareVersionMajor;
}

int Lpf2Hub::getFirmwareVersionMinor()
{
    return Lpf2HubFirmwareVersionMinor;
}

int Lpf2Hub::getHardwareVersionBuild()
{
    return Lpf2HubHardwareVersionBuild;
}

int Lpf2Hub::getHardwareVersionBugfix()
{
    return Lpf2HubHardwareVersionBugfix;
}

int Lpf2Hub::getHardwareVersionMajor()
{
    return Lpf2HubHardwareVersionMajor;
}

int Lpf2Hub::getHardwareVersionMinor()
{
    return Lpf2HubHardwareVersionMinor;
}

bool Lpf2Hub::isButtonPressed()
{
    return Lpf2HubHubButtonPressed;
}

bool Lpf2Hub::isLeftRemoteUpButtonPressed()
{
    return Lpf2HubRemoteLeftUpButtonPressed;
}

bool Lpf2Hub::isLeftRemoteDownButtonPressed()
{
    return Lpf2HubRemoteLeftDownButtonPressed;
}

bool Lpf2Hub::isLeftRemoteStopButtonPressed()
{
    return Lpf2HubRemoteLeftStopButtonPressed;
}

bool Lpf2Hub::isLeftRemoteButtonReleased()
{
    return Lpf2HubRemoteLeftButtonReleased;
}

bool Lpf2Hub::isRightRemoteUpButtonPressed()
{
    return Lpf2HubRemoteRightUpButtonPressed;
}

bool Lpf2Hub::isRightRemoteDownButtonPressed()
{
    return Lpf2HubRemoteRightDownButtonPressed;
}

bool Lpf2Hub::isRightRemoteStopButtonPressed()
{
    return Lpf2HubRemoteRightStopButtonPressed;
}

bool Lpf2Hub::isRightRemoteButtonReleased()
{
    return Lpf2HubRemoteRightButtonReleased;
}
