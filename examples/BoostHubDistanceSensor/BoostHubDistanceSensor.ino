/**
 * A BoostHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected distance of the distance/color sensor
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "BoostHub.h"

// create a hub instance
BoostHub myBoostHub;
BoostHub::Port _portC = BoostHub::Port::C;

// callback function to handle updates of sensor values
void distanceSensorCallback(byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR)
  {
    double distance = myBoostHub.parseDistance(pData);
    Serial.print("Distance: ");
    Serial.println(distance, DEC);
    // set hub LED color dependent on the distance of the sensor to an object
    // red -- short distance
    // orange -- medium distance
    // green -- large distance
    if (distance < 40.0)
    {
      myBoostHub.setLedColor(RED);
    }
    else if (distance < 80.0 && distance >= 40.0)
    {
      myBoostHub.setLedColor(ORANGE);
    }
    else
    {
      myBoostHub.setLedColor(GREEN);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  myBoostHub.init(); // initalize the BoostHub instance
}

// main loop
void loop()
{

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myBoostHub.isConnecting())
  {
    myBoostHub.connectHub();
    if (myBoostHub.isConnected())
    {
      Serial.println("Connected to HUB");
      delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
      // connect color/distance sensor to port c, activate sensor for updates, set callback function if distance value changes
      myBoostHub.activatePortDevice(_portC, (byte)DeviceType::COLOR_DISTANCE_SENSOR, distanceSensorCallback);
      myBoostHub.setLedColor(GREEN);
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myBoostHub.isConnected())
  {
  }

} // End of loop
