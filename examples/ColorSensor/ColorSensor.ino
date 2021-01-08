/**
 * A Hub basic example to connect a hub, set the led color of the hub 
 * dependent on the detected color of the color/distance sensor which 
 * could be attached to any free port. The sketch will wait till a color
 * sensor ist attached to a port. 
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myHub;
bool isInitialized = false;

void setup()
{
  Serial.begin(115200);
  myHub.init(); // initalize the hub instance
}

// callback function to handle updates of sensor values
void colorDistanceSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.print(portNumber, DEC);
  Serial.print(" value: ");
  Serial.println(pData[4], DEC);
  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR)
  {
    int color = myHub->parseColor(pData);
    double distance = myHub->parseDistance(pData);
    Serial.print("Color: ");
    Serial.print(LegoinoCommon::ColorStringFromColor(color).c_str());
    Serial.print(" Distance: ");
    Serial.println(distance, DEC);
    myHub->setLedColor((Color)color);
  }
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
      myHub.setLedColor(BLUE);
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
    byte portForDevice = myHub.getPortForDeviceType((byte)DeviceType::COLOR_DISTANCE_SENSOR);
    Serial.println(portForDevice, DEC);
    if (portForDevice != 255)
    {
      Serial.println("activatePortDevice");
      myHub.activatePortDevice(portForDevice, colorDistanceSensorCallback);
      delay(200);
      myHub.setLedColor(GREEN);
      isInitialized = true;
    };
  }

} // End of loop
