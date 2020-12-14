/**
 * A hub basic example to connect a  hub, set the led color of the hub 
 * dependent on the detected distance of the distance/color sensor. The sensor
 * can be connect to any port. The sketch will wait till a color-distance sensor
 * will be attached on any port.
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myHub;
bool isInitialized = false;

// callback function to handle updates of sensor values
void distanceSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.print(portNumber, DEC);
  Serial.print(" value: ");
  Serial.println(pData[7], DEC);
  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR)
  {
    double distance = myHub->parseDistance(pData);
    Serial.print("Distance: ");
    Serial.println(distance, DEC);
    // set hub LED color dependent on the distance of the sensor to an object
    // red -- short distance
    // orange -- medium distance
    // green -- large distance
    if (distance < 40.0)
    {
      myHub->setLedColor(RED);
    }
    else if (distance < 80.0 && distance >= 40.0)
    {
      myHub->setLedColor(ORANGE);
    }
    else
    {
      myHub->setLedColor(GREEN);
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
      myHub.activatePortDevice(portForDevice, distanceSensorCallback);
      delay(200);
      myHub.setLedColor(GREEN);
      isInitialized = true;
    };
  }

} // End of loop
