/**
 * A MoveHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected distance of the distance/color sensor
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myMoveHub;
byte portC = (byte)MoveHubPort::C;

// callback function to handle updates of sensor values
void distanceSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
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
  myMoveHub.init(); // initalize the MoveHub instance
}

// main loop
void loop()
{

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myMoveHub.isConnecting())
  {
    myMoveHub.connectHub();
    if (myMoveHub.isConnected())
    {
      Serial.println("Connected to HUB");
      delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
      // connect color/distance sensor to port c, activate sensor for updates, set callback function if distance value changes
      myMoveHub.activatePortDevice(portC, distanceSensorCallback);
      myMoveHub.setLedColor(GREEN);
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myMoveHub.isConnected())
  {
  }

} // End of loop
