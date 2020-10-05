/**
 * A Train hub basic example to connect a PoweredUp hub, and set the speed of the train 
 * motor dependent on the detected color. If the train is stopped, you can start the train by
 * pressing the hub button.
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpHub.h"

// create a hub instance
PoweredUpHub myHub;
PoweredUpHub::Port _portA = PoweredUpHub::Port::A;
PoweredUpHub::Port _portB = PoweredUpHub::Port::B;

oid hubButtonCallback(HubPropertyReference hubProperty, uint8_t *pData)
{
  if (hubProperty == HubPropertyReference::BUTTON)
  {
    ButtonState buttonState = myHub.parseHubButton(pData);
    Serial.print("Button: ");
    Serial.println((byte)buttonState, HEX);

    if (buttonState == ButtonState::PRESSED)
    {
      myHub.setMotorSpeed(_portA, 15);
    }
  }
}

// callback function to handle updates of sensor values
void colorDistanceSensorCallback(byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR)
  {
    int color = myHub.parseColor(pData);
    Serial.print("Color: ");
    Serial.println(COLOR_STRING[color]);

    // set hub LED color to detected color of sensor and set motor speed dependent on color
    if (color == (byte)Color::RED)
    {
      myHub.setLedColor(color);
      myHub.stopMotor(_portA);
    }
    else if (color == (byte)Color::YELLOW)
    {
      myHub.setLedColor(color);
      myHub.setMotorSpeed(_portA, 25);
    }
    else if (color == (byte)Color::BLUE)
    {
      myHub.setLedColor(color);
      myHub.setMotorSpeed(_portA, 35);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  myHub.init(); // initalize the PoweredUp hub instance
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
      delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
      // connect color/distance sensor to port c, activate sensor for updates
      myHub.activatePortDevice(_portB, colorDistanceSensorCallback);
      // activate hub button to receive updates
      myHub.activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);
      myHub.setLedColor(GREEN);
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }


} // End of loop
