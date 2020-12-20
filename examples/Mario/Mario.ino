/**
 *  TODO
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

DeviceType pantSensor = DeviceType::MARIO_HUB_PANT_SENSOR;
DeviceType gestureSensor = DeviceType::MARIO_HUB_GESTURE;
DeviceType barcodeSensor = DeviceType::MARIO_HUB_BARCODE_SENSOR;


// create a hub instance
Lpf2Hub myHub;
bool isInitialized = false;

void MarioCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, HEX);
  if (deviceType == DeviceType::MARIO_HUB_PANT_SENSOR)
  {
    MarioPant pant = myHub->parseMarioPant(pData);
    Serial.print("Mario Pant: ");
    Serial.println((byte)pant, DEC);
  }
  if (deviceType == DeviceType::MARIO_HUB_BARCODE_SENSOR)
  {
    MarioBarcode barcode = myHub->parseMarioBarcode(pData);
    Serial.print("Mario Barcode: ");
    Serial.println((byte)barcode, HEX);
    MarioColor color = myHub->parseMarioColor(pData);
    Serial.print("Mario Color: ");
    Serial.println((byte)color, HEX);
  }
}

void setup()
{
  Serial.begin(115200);
  myHub.init(); // initalize the MoveHub instance
}

// main loop
void loop()
{

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myHub.isConnecting())
  {
    myHub.connectHub();
    if (myHub.isConnected())
    {
      Serial.println("Connected to HUB");
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  if (myHub.isConnected() && !isInitialized)
  {
    delay(200);
    Serial.print("check ports... if needed sensor is already connected: ");
    byte portForDevice = myHub.getPortForDeviceType((byte)barcodeSensor);
    Serial.println(portForDevice, DEC);
    if (portForDevice != 255)
    {
      Serial.println("activatePortDevice");
      myHub.activatePortDevice(portForDevice, MarioCallback);
      delay(200);
      isInitialized = true;
    };
  }

} // End of loop
