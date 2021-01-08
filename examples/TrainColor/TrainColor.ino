/**
 * A Train hub basic example to connect a PoweredUp hub, and set the speed of the train 
 * motor dependent on the detected color. If the train is stopped, you can start the train by
 * pressing the hub button. The train motor has to be connected to Port A and the Color sensor to Port B
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myHub;
byte portA = (byte)PoweredUpHubPort::A;
byte portB = (byte)PoweredUpHubPort::B;

bool isInitialized = false;

void hubButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    ButtonState buttonState = myHub->parseHubButton(pData);
    Serial.print("Button: ");
    Serial.println((byte)buttonState, HEX);

    if (buttonState == ButtonState::PRESSED)
    {
      myHub->setBasicMotorSpeed(portA, 15);
    }
  }
}

// callback function to handle updates of sensor values
void colorDistanceSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR)
  {
    int color = myHub->parseColor(pData);
    Serial.print("Color: ");
    Serial.println(LegoinoCommon::ColorStringFromColor(color).c_str());

    // set hub LED color to detected color of sensor and set motor speed dependent on color
    if (color == (byte)Color::RED)
    {
      myHub->setLedColor((Color)color);
      myHub->stopBasicMotor(portA);
    }
    else if (color == (byte)Color::YELLOW)
    {
      myHub->setLedColor((Color)color);
      myHub->setBasicMotorSpeed(portA, 25);
    }
    else if (color == (byte)Color::BLUE)
    {
      myHub->setLedColor((Color)color);
      myHub->setBasicMotorSpeed(portA, 35);
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
      myHub.setLedColor(RED);
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
      myHub.activatePortDevice(portB, colorDistanceSensorCallback);
      delay(200);
      // activate hub button to receive updates
      myHub.activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);
      delay(200);
      myHub.setLedColor(GREEN);
      isInitialized = true;
    };

  }

} // End of loop
