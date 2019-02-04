
#include "Legoino.h"

HubType _hubType;
BLEUUID _bleUuid;
BLEUUID _charachteristicUuid;
BLEAddress *_pServerAddress;
BLERemoteCharacteristic *_pRemoteCharacteristic;
boolean _isConnecting = false;
boolean _isConnected = false;


/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(_bleUuid)) {

      // 
      Serial.print("Found our device!  address: "); 
      advertisedDevice.getScan()->stop();

      _pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      _isConnecting = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

Legoino::Legoino()
{

};


void Legoino::init(HubType hubType) {
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
    _hubType = hubType;
    if (hubType == WEDO) {
        _bleUuid = BLEUUID(WEDO_UUID);
        _charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
    } else {
        _bleUuid = BLEUUID(LPF2_UUID);
        _charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
    };

  Serial.begin(115200);
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void Legoino::setLedColor(Color color) {
    byte setColor[8] = {0x08, 0x00, 0x81, 0x32, 0x11, 0x51, 0x00, color};
    _pRemoteCharacteristic->writeValue(setColor, sizeof(setColor));
}

void Legoino::setMotorSpeed(Port port, byte speed) {
    
    if (speed == 0) {
       stopMotor(port);
       return;
    }

    byte rawSpeed = 127; // stop motor
    if (speed > 0) {
        rawSpeed = map(speed, 0, 100, 0, 126);
    } else {
        rawSpeed = map(speed, -100, 0, 128, 255);
    }
    byte setMotorCommand[8] = {0x08, port, 0x81, 0x00, 0x11, 0x51, 0x00, rawSpeed};
    _pRemoteCharacteristic->writeValue(setMotorCommand, sizeof(setMotorCommand), false);
    
}

void Legoino::stopMotor(Port port=AB) {
    byte setMotorCommand[8] = {0x08, port, 0x81, 0x00, 0x11, 0x51, 0x00, 0x7F};
    _pRemoteCharacteristic->writeValue(setMotorCommand, sizeof(setMotorCommand), false);  
}

bool Legoino::connectHub() {
    BLEAddress pAddress = *_pServerAddress;
    Serial.print("Forming a connection to ");
    Serial.println(pAddress.toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    // Connect to the remove BLE Server.
    pClient->connect(pAddress);
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(_bleUuid);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(_bleUuid.toString().c_str());
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    _pRemoteCharacteristic = pRemoteService->getCharacteristic(_charachteristicUuid);
    if (_pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(_charachteristicUuid.toString().c_str());
      return false;
    }
    Serial.println(" - Found our characteristic");
    _isConnected = true;
    _isConnecting = false;
    return true;
}

bool Legoino::isConnecting() {
    return _isConnecting;
}

bool Legoino::isConnected() {
    return _isConnected;
}