/**
 * A hub basic example to connect a hub, set the led color of the hub 
 * dependent on the detected rotation of the connected tacho motor. Usage of callback function
 * if motor angle is changed. The tacho motor can be attached to any port and the 
 * sketch will wait until the tacho motor is attached. If the hub button is pressed,
 * the encode position will be set to 0
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// Define here the linear/angular motor which you will connect to a port of your hub
DeviceType device = DeviceType::MEDIUM_LINEAR_MOTOR;

// create a hub instance
Lpf2Hub myHub;
byte port;
bool isInitialized = false;

void buttonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    if (myHub->parseHubButton(pData) == ButtonState::PRESSED) {
      myHub->setAbsoluteMotorEncoderPosition(port, 0);
    }
  }
}

// callback function to handle updates of sensor values
void tachoMotorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::MEDIUM_LINEAR_MOTOR)
  {
    int rotation = myHub->parseTachoMotor(pData);
    Serial.print("Rotation: ");
    Serial.print(rotation, DEC);
    Serial.println(" [degrees]");
    myHub->setLedHSVColor(abs(rotation), 1.0, 1.0);
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
      delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
      myHub.activateHubPropertyUpdate(HubPropertyReference::BUTTON, buttonCallback);
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
    byte portForDevice = myHub.getPortForDeviceType((byte)device);
    Serial.println(portForDevice, DEC);
    if (portForDevice != 255)
    {
      port = portForDevice;
      Serial.println("activatePortDevice");
      myHub.activatePortDevice(portForDevice, tachoMotorCallback);
      delay(200);
      myHub.setLedColor(GREEN);
      isInitialized = true;
    };
  }


} // End of loop
