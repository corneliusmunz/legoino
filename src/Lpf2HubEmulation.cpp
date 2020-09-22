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

    void onConnect(NimBLEServer* pServer) {
      LOGLINE("Device connected");
            delay(1000);
            LOGLINE("Send Hub port configuration");
            
            //Boost motor on Port_A
            byte PORT_A_INFORMATION[]={0x0F, 0x00, 0x04, 0x00, 0x01, 0x26, 0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10};
            _lpf2HubEmulation->_pCharacteristic->setValue(PORT_A_INFORMATION,15);
            _lpf2HubEmulation->_pCharacteristic->notify();
            delay(100);
            //Boost motor on Port_B
            byte PORT_B_INFORMATION[]={0x0F, 0x00, 0x04, 0x01, 0x01, 0x26, 0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10};
            _lpf2HubEmulation->_pCharacteristic->setValue(PORT_B_INFORMATION,15);
            _lpf2HubEmulation->_pCharacteristic->notify();
            delay(100);

            //Led
            byte PORT_LIGHT_INFORMATION[]={0x0F, 0x00, 0x04, 0x32, 0x01, 0x17, 0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10};
            _lpf2HubEmulation->_pCharacteristic->setValue(PORT_LIGHT_INFORMATION,15);
            _lpf2HubEmulation->_pCharacteristic->notify();

            delay(50);
    };

    void onDisconnect(NimBLEServer* pServer) {
      LOGLINE("Device disconnected");
    }
};

class Lpf2HubCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    
    Lpf2HubEmulation *_lpf2HubEmulation;
    
public:    
    Lpf2HubCharacteristicCallbacks(Lpf2HubEmulation *lpf2HubEmulation) : NimBLECharacteristicCallbacks()
    {
        _lpf2HubEmulation = lpf2HubEmulation;
    }

    void onWrite(NimBLECharacteristic *pCharacteristic) {
      
      std::string msgReceived = pCharacteristic->getValue();

      if (msgReceived.length() > 0) {
        LOG("Message received ");
        LOG(msgReceived.length());
        LOG(" bytes :");
        for (int i = 2; i < msgReceived.length(); i++){
          if (i==MSG_TYPE){
            LOG(" MSG_TYPE > ");
          } else if (i == (MSG_TYPE + 1)){
            LOG(" PAYLOAD > ");
          }
           
          LOG("0x");
          LOG(msgReceived[i], HEX);
          LOG("(");
          LOG(msgReceived[i],DEC);
          LOG(")");
          LOG(" ");
         } 
         LOGLINE("");

            //It's a port out command:
      //execute and send feedback to the App
      if (msgReceived[MSG_TYPE]==OUT_PORT_CMD){
        LOGLINE("Port command received");
        delay(30);

        //Reply to the App "Command excecuted"
        byte msgPortCommandFeedbackReply[]={0x05, 0x00, 0x82, 0x00, 0x0A}; //0x0A Command complete+buffer empty+idle
        msgPortCommandFeedbackReply[PORT_ID]=msgReceived[PORT_ID]; //set the port_id
        _lpf2HubEmulation->_pCharacteristic->setValue(msgPortCommandFeedbackReply,sizeof(msgPortCommandFeedbackReply));
        _lpf2HubEmulation->_pCharacteristic->notify();
      }
    }
    }

    void onRead(NimBLECharacteristic *pCharacteristic) {
      LOGLINE("Read request");
      uint8_t CharTemp[]={0x0F, 0x00, 0x04};
      _lpf2HubEmulation->_pCharacteristic->setValue(CharTemp,3);
    }


};

Lpf2HubEmulation::Lpf2HubEmulation(){};

void Lpf2HubEmulation::start() {
    LOGLINE("Starting BLE work!");

    uint8_t newMACAddress[] = {0x90, 0x84, 0x2B, 0x4A, 0x3A, 0x0A};
    esp_base_mac_addr_set(&newMACAddress[0]);
    NimBLEDevice::init("Fake Hub");

    LOGLINE("Create server");
    _pServer = NimBLEDevice::createServer();
    _pServer->setCallbacks(new Lpf2HubServerCallbacks(this));

    LOGLINE("Create service");
    _pService = _pServer->createService(SERVICE_UUID);

   // Create a BLE Characteristic
  _pCharacteristic = _pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::READ   |
                      NIMBLE_PROPERTY::WRITE  |
                      NIMBLE_PROPERTY::NOTIFY |
                      NIMBLE_PROPERTY::WRITE_NR
                    );
  // Create a BLE Descriptor and set the callback
  _pCharacteristic->setCallbacks(new Lpf2HubCharacteristicCallbacks(this));

  LOGLINE("Service start");

  _pService->start();
  _pAdvertising = NimBLEDevice::getAdvertising();

  _pAdvertising->addServiceUUID(SERVICE_UUID);
  _pAdvertising->setScanResponse(true);

  //Techinc HUB
  const char  ArrManufacturerData[8] = {0x97,0x03,0x00,0x80,0x06,0x00,0x41,0x00};

  //City HUB
  //const char  ArrManufacturerData[8] = {0x97,0x03,0x00,0x41,0x07,0x3E,0x3F,0x00};
  std::string ManufacturerData(ArrManufacturerData ,sizeof(ArrManufacturerData));
  char advLEGO[] = {0x02,0x01,0x06,0x11,0x07,0x23,0xD1,0xBC,0xEA,0x5F,0x78,0x23,0x16,0xDE,0xEF,
                          0x12,0x12,0x23,0x16,0x00,0x00,0x09,0xFF,0x97,0x03,0x00,0x41,0x07,0xB2,0x43,0x00};
  
  NimBLEAdvertisementData oScanResponseData = NimBLEAdvertisementData();
  oScanResponseData.addData(advLEGO, 31);
  // oScanResponseData.setShortName("Fake Hub");
  // oScanResponseData.setManufacturerData(ManufacturerData);
  //oScanResponseData.addData(advLEGO, 31);
  std::string payload = oScanResponseData.getPayload();
  LOG("AdvertisementData payload: ");
  for (int i=0; i<payload.length(); i++) {
    LOG(" ");
    LOG(payload[i], HEX);
  }
  LOGLINE("");
  _pAdvertising->setScanResponseData(oScanResponseData);
  _pAdvertising->setAdvertisementData(oScanResponseData);



  LOGLINE("Start adv");
  NimBLEDevice::startAdvertising();
  LOGLINE("Characteristic defined! Now you can read it in your phone!");
}


