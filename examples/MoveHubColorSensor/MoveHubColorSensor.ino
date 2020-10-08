/**
 * A MoveHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected color of the color/distance sensor. 
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "MoveHub.h"

// create a hub instance
MoveHub myMoveHub;
MoveHub::Port _portC = MoveHub::Port::C;

void setup()
{
  Serial.begin(115200);
  myMoveHub.init(); // initalize the MoveHub instance
}

// callback function to handle updates of sensor values
void colorDistanceSensorCallback(byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR)
  {
    int color = myMoveHub.parseColor(pData);
    double distance = myMoveHub.parseDistance(pData);
    Serial.print("Color: ");
    Serial.print(COLOR_STRING[color]);
    Serial.print(" Distance: ");
    Serial.println(distance, DEC);
    myMoveHub.setLedColor((Color)color);
  }
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
      // connect color/distance sensor to port c, activate sensor for updates
      myMoveHub.activatePortDevice(_portC, colorDistanceSensorCallback);
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
    // nothing has to be done because the sensor values are received in the callback function if an update occurs
  }

} // End of loop
