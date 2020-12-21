/**
 * 
 * A basic example which will connect to a Mario hub and request updates for 
 * the Pant, Color/Barcode and Gesture sensor. 
 *  
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

DeviceType pantSensor = DeviceType::MARIO_HUB_PANT_SENSOR;
DeviceType gestureSensor = DeviceType::MARIO_HUB_GESTURE_SENSOR;
DeviceType barcodeSensor = DeviceType::MARIO_HUB_BARCODE_SENSOR;

bool isPantSensorInitialized = false;
bool isGestureSensorInitialized = false;
bool isBarcodeSensorInitialized = false;

// create a hub instance
Lpf2Hub myHub;

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
  if (deviceType == DeviceType::MARIO_HUB_GESTURE_SENSOR)
  {
    MarioGesture gesture = myHub->parseMarioGesture(pData);
    if (gesture != MarioGesture::NONE) // filter out NONE values
    {
      Serial.print("Mario Gesture: ");
      Serial.println((int)gesture, HEX);
    }
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

  // connect flow. Search for BLE services and try to connect
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

  if (myHub.isConnected() && !isGestureSensorInitialized)
  {
    delay(200);
    Serial.print("check ports... if needed sensor is already connected: ");
    byte portForDevice = myHub.getPortForDeviceType((byte)gestureSensor);
    Serial.println(portForDevice, DEC);
    if (portForDevice != 255)
    {
      Serial.println("activatePortDevice");
      myHub.activatePortDevice(portForDevice, MarioCallback);
      delay(200);
      isGestureSensorInitialized = true;
    };
  }

  if (myHub.isConnected() && !isPantSensorInitialized)
  {
    delay(200);
    Serial.print("check ports... if needed sensor is already connected: ");
    byte portForDevice = myHub.getPortForDeviceType((byte)pantSensor);
    Serial.println(portForDevice, DEC);
    if (portForDevice != 255)
    {
      Serial.println("activatePortDevice");
      myHub.activatePortDevice(portForDevice, MarioCallback);
      delay(200);
      isPantSensorInitialized = true;
    };
  }

  if (myHub.isConnected() && !isBarcodeSensorInitialized)
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
      isBarcodeSensorInitialized = true;
    };
  }  

} // End of loop
