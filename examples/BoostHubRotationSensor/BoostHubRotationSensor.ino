/**
 * A BoostHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected rotation of the boost tacho motor. Usage of callback function
 * if motor angle is changed
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "BoostHub.h"

// create a hub instance
BoostHub myBoostHub;
BoostHub::Port _portD = BoostHub::Port::D;

// callback function to handle updates of sensor values
void tachoMotorCallback(byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::MEDIUM_LINEAR_MOTOR)
  {
    int rotation = myBoostHub.parseTachoMotor(pData);
    Serial.print("Rotation: ");
    Serial.print(rotation, DEC);
    Serial.println(" [degrees]");
    myBoostHub.setLedHSVColor(abs(rotation), 1.0, 1.0);
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
      // connect boost tacho motor  to port d, activate sensor for updates, set callback function for rotation changes
      myBoostHub.activatePortDevice(_portD, tachoMotorCallback);
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
