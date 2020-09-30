/**
 * A Legoino example to connect to a Powered Up remote and a train hub. 
 * 
 * 1) Power up the ESP32
 * 2) Power up the Remote and the Train Hub in any order
 * 
 * You can change the motor speed with the left (A) remote buttons
 * 
 * (c) Copyright 2020 - Nicolas HILAIRE
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"
#include "PoweredUpHub.h"

// create a hub instance
PoweredUpRemote myRemote;
PoweredUpHub myHub;

PoweredUpRemote::Port _portLeft = PoweredUpRemote::Port::LEFT;
PoweredUpRemote::Port _portRight = PoweredUpRemote::Port::RIGHT;
PoweredUpHub::Port _portA = PoweredUpHub::Port::A;

int currentSpeed = 0;
int updatedSpeed = 0;
bool isInitialized = false;

// callback function to handle updates of remote buttons
void remoteCallback(byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON)
  {
    ButtonState buttonState = myRemote.parseRemoteButton(pData);
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
      myHub.setMotorSpeed(_portA, updatedSpeed);
      currentSpeed = updatedSpeed;
    }

    Serial.print("Current speed:");
    Serial.println(currentSpeed, DEC);
  }
}

void setup()
{
  Serial.begin(115200);
  myRemote.init(); // initialize the remote hub
  myHub.init();    // initialize the listening hub
}

// main loop
void loop()
{

  //wait for two elements

  if (myRemote.isConnecting())
  {
    if (myRemote.getHubType() == HubType::POWERED_UP_REMOTE)
    {
      //This is the right device
      if (!myRemote.connectHub())
      {
        Serial.println("Unable to connect to hub");
      }
      else
      {
        myRemote.setLedColor(GREEN);
        Serial.println("Remote connected.");
      }
    }
  }

  if (myHub.isConnecting())
  {
    if (myHub.getHubType() == HubType::POWERED_UP_HUB)
    {
      myHub.connectHub();
      myHub.setLedColor(GREEN);
      Serial.println("powered up hub connected.");
    }
  }

  if (!myRemote.isConnected())
  {
    myRemote.init();
  }

  if (!myHub.isConnected())
  {
    myHub.init();
  }

  if (myRemote.isConnected() && myHub.isConnected() && !isInitialized)
  {
    Serial.println("System is initialized");
    isInitialized = true;
    delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
    // both activations are needed to get status updates
    myRemote.activatePortDevice(_portLeft, (byte)DeviceType::REMOTE_CONTROL_BUTTON, remoteCallback);
    myRemote.activatePortDevice(_portRight, (byte)DeviceType::REMOTE_CONTROL_BUTTON, remoteCallback);
    myRemote.setLedColor(WHITE);
    myHub.setLedColor(WHITE);
  }


} // End of loop