/**
 * A Legoino example to connect to a Powered Up remote and a train hub. 
 * 
 * ATTENTION: The connection order is relevant!
 * 1) Power up the ESP32
 * 2) Power up the Remote
 * 3) Power up the Train Hub
 * 
 * You can change the motor speed with the left (A) remote buttons
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myRemote;
Lpf2Hub myHub;

byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
byte portA = (byte)PoweredUpHubPort::A;

int currentSpeed = 0;
int updatedSpeed = 0;
bool isInitialized = false;

// callback function to handle updates of remote buttons
void remoteCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myRemoteHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON)
  {
    ButtonState buttonState = myRemoteHub->parseRemoteButton(pData);
    Serial.print("Buttonstate: ");
    Serial.println((byte)buttonState, HEX);

    if (buttonState == ButtonState::UP)
    {
      updatedSpeed = min(100, currentSpeed + 10);
    }
    else if (buttonState == ButtonState::DOWN)
    {
      updatedSpeed = max(-100, currentSpeed - 10);
    }
    else if (buttonState == ButtonState::STOP)
    {
      updatedSpeed = 0;
    }

    if (currentSpeed != updatedSpeed)
    {
      myHub.setBasicMotorSpeed(portA, updatedSpeed);
      currentSpeed = updatedSpeed;
    }

    Serial.print("Current speed:");
    Serial.println(currentSpeed, DEC);
  }
}

void setup()
{
  Serial.begin(115200);
  myRemote.init(); // initalize the remote instance and try to connect
}

// main loop
void loop()
{

  // connect flow
  if (myRemote.isConnecting())
  {
    myRemote.connectHub();
    if (myRemote.isConnected())
    {
      Serial.println("Connected to Remote");
      myRemote.setLedColor(GREEN);
      myHub.init(); // after connecting the remote, try to connect the hub
    }
    else
    {
      Serial.println("Failed to connect to Remote");
    }
  }

  if (myHub.isConnecting())
  {
    myHub.connectHub();
    if (myHub.isConnected())
    {
      Serial.println("Connected to Hub");
    }
    else
    {
      Serial.println("Failed to connect to Hub");
    }
  }

  if (myRemote.isConnected() && myHub.isConnected() && !isInitialized)
  {
    Serial.println("System is initialized");
    isInitialized = true;
    delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
    // both activations are needed to get status updates
    myRemote.activatePortDevice(portLeft, remoteCallback);
    myRemote.activatePortDevice(portRight, remoteCallback);
  }

} // End of loop
