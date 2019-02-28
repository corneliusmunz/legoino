/*
 * Lpf2Hub.cpp - Arduino base class for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#include "Lpf2Hub.h"

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class Lpf2HubAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    Lpf2Hub* _lpf2Hub;
public:
    Lpf2HubAdvertisedDeviceCallbacks(Lpf2Hub* lpf2Hub) : BLEAdvertisedDeviceCallbacks() {
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
void Lpf2Hub::WriteValue(byte command[], int size) { 
    byte commandWithCommonHeader[size+2] = {size+2, 0x00};
    memcpy(commandWithCommonHeader+2, command, size);
    _pRemoteCharacteristic->writeValue(commandWithCommonHeader, sizeof(commandWithCommonHeader), false);
}


/**
 * @brief Map speed from -100..100 to the 8bit internal value
 * @param [in] speed -100..100
 */
byte Lpf2Hub::MapSpeed(int speed) {
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

byte* Lpf2Hub::Int16ToByteArray(int16_t x) {
    static byte y[2];
    y[0] = (byte) (x & 0xff);
    y[1] = (byte) ((x >> 8) & 0xff);
    return y;
}

byte* Lpf2Hub::Int32ToByteArray(int32_t x) {
    static byte y[4];
    y[0] = (byte) (x & 0xff);
    y[1] = (byte) ((x >> 8) & 0xff);
    y[2] = (byte) ((x >> 16) & 0xff);
    y[3] = (byte) ((x >> 24) & 0xff);
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

/**
 * @brief Parse the incoming characteristic notification for a Device Info Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parseDeviceInfo(uint8_t *pData)
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
void Lpf2Hub::parsePortMessage(uint8_t *pData)
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

}

/**
 * @brief Parse the incoming characteristic notification for a Sensor Message
 * @param [in] pData The pointer to the received data
 */
void Lpf2Hub::parseSensorMessage(uint8_t *pData)
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
void Lpf2Hub::parsePortAction(uint8_t *pData)
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
void Lpf2Hub::notifyCallback(
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



Lpf2Hub::Lpf2Hub(){};

/**
 * @brief Init function set the UUIDs and scan for the Hub
 */
void Lpf2Hub::init(HubType hubType)
{
    _isConnected=false;
    _isConnecting=false;
    _bleUuid = BLEUUID(LPF2_UUID);
    _charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
    _hubType = hubType;

    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();
        Serial.println("setAvertisedDeviceCallbacks");

    pBLEScan->setAdvertisedDeviceCallbacks(new Lpf2HubAdvertisedDeviceCallbacks(this));
        Serial.println("activate scanning");

    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
            Serial.println("start");


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

    memcpy(setNameCommand+offset, name, nameLength);
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

    byte setNameCommand[3] = {0x01, 0x01, 0x02};
    WriteValue(setNameCommand, 3);

    byte setFWCommand[3] = {0x01, 0x03, 0x02};
    WriteValue(setFWCommand, 3);
    
    byte setHWCommand[3] = {0x01, 0x04, 0x02};
    WriteValue(setHWCommand, 3);
    
    byte setBatteryType[3] = {0x01, 0x07, 0x02};
    WriteValue(setBatteryType, 3);
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

    Serial.println("get pClient");
    BLERemoteService *pRemoteService = pClient->getService(_bleUuid);
    if (pRemoteService == nullptr)
    {
            Serial.println("get pClient failed");
        return false;
    }
        Serial.println("get pRemoteService");

    _pRemoteCharacteristic = pRemoteService->getCharacteristic(_charachteristicUuid);
    if (_pRemoteCharacteristic == nullptr)
    {
            Serial.println("get pRemoteService failed");

        return false;
    }

    // register notifications (callback function) for the characteristic
    if (_pRemoteCharacteristic->canNotify())
    {
        //_pRemoteCharacteristic->registerForNotify(notifyCallback);
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