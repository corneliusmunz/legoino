/**
 * A BoostHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected color of the color/distance sensor. 
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "BoostHub.h"

// create a hub instance
BoostHub myBoostHub;
BoostHub::Port _portC = BoostHub::Port::C;

void setup()
{
  Serial.begin(115200);
  myBoostHub.init(); // initalize the BoostHub instance
}

// callback function to handle updates of sensor values
void colorDistanceSensorCallback(byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR)
  {
    int color = myBoostHub.parseColor(pData);
    double distance = myBoostHub.parseDistance(pData);
    Serial.print("Color: ");
    Serial.print(COLOR_STRING[color]);
    Serial.print(" Distance: ");
    Serial.println(distance, DEC);
    myBoostHub.setLedColor((Color)color);
  }
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
      // connect color/distance sensor to port c, activate sensor for updates
      myBoostHub.activatePortDevice(_portC, colorDistanceSensorCallback);
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
    // nothing has to be done because the sensor values are received in the callback function if an update occurs
  }

} // End of loop
