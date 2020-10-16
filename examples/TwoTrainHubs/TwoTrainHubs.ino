/**
 * A Legoino example to connect two train hubs and one powered up remote. 
 * 
 * 1) Power up the ESP32
 * 2) Power up the Remote and the Train Hubs in any order
 * 
 * You can change the motor speed of train 1 with the left (A) remote buttons
 * You can change the motor speed of train 2 with the right (B) remote buttons
 * 
 * (c) Copyright 2020
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myRemote;
Lpf2Hub myTrainHub1;
Lpf2Hub myTrainHub2;

byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
byte portA = (byte)PoweredUpHubPort::A;

int currentSpeedTrain1 = 0;
int currentSpeedTrain2 = 0;
int updatedSpeedTrain1 = 0;
int updatedSpeedTrain2 = 0;

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

    // Do the logic for left buttons of remote control and Train Hub 1
    if (portNumber == (byte)portLeft && buttonState == ButtonState::UP)
    {
      updatedSpeedTrain1 = min(100, currentSpeedTrain1 + 10);
    }
    else if (portNumber == (byte)portLeft && buttonState == ButtonState::DOWN)
    {
      updatedSpeedTrain1 = min(100, currentSpeedTrain1 - 10);
    }
    else if (portNumber == (byte)portLeft && buttonState == ButtonState::STOP)
    {
      updatedSpeedTrain1 = 0;
    }

    if (currentSpeedTrain1 != updatedSpeedTrain1)
    {
      myTrainHub1.setBasicMotorSpeed(portA, updatedSpeedTrain1);
      currentSpeedTrain1 = updatedSpeedTrain1;
    }

    Serial.print("Current speed train 1:");
    Serial.println(currentSpeedTrain1, DEC);

    // Do the logic for right buttons of remote control and Train Hub 2
    if (portNumber == (byte)portRight && buttonState == ButtonState::UP)
    {
      updatedSpeedTrain2 = min(100, currentSpeedTrain2 + 10);
    }
    else if (portNumber == (byte)portRight && buttonState == ButtonState::DOWN)
    {
      updatedSpeedTrain2 = min(100, currentSpeedTrain2 - 10);
    }
    else if (portNumber == (byte)portRight && buttonState == ButtonState::STOP)
    {
      updatedSpeedTrain2 = 0;
    }

    if (currentSpeedTrain2 != updatedSpeedTrain2)
    {
      myTrainHub2.setBasicMotorSpeed(portA, updatedSpeedTrain2);
      currentSpeedTrain2 = updatedSpeedTrain2;
    }

    Serial.print("Current speed train 2:");
    Serial.println(currentSpeedTrain2, DEC);
  }
}

void setup()
{
  Serial.begin(115200);
  myRemote.init();                       // initialize the remote control hub
  myTrainHub1.init("90:84:2b:03:19:7f"); // initialize the listening train hub 1 // here you have to use your own device ids
  myTrainHub2.init("90:84:2b:06:76:a6"); // initialize the listening train hub 2 // here you have to use your own device ids
}

// main loop
void loop()
{

  //wait for three elements

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

  if (myTrainHub1.isConnecting())
  {
    if (myTrainHub1.getHubType() == HubType::POWERED_UP_HUB)
    {
      myTrainHub1.connectHub();
      myTrainHub1.setLedColor(BLUE);
      Serial.println("train hub 1 connected.");
    }
  }

  if (myTrainHub2.isConnecting())
  {
    if (myTrainHub2.getHubType() == HubType::POWERED_UP_HUB)
    {
      myTrainHub2.connectHub();
      myTrainHub2.setLedColor(YELLOW);
      Serial.println("train hub 2 connected.");
    }
  }

  if (!myRemote.isConnected())
  {
    myRemote.init();
  }

  if (!myTrainHub1.isConnected())
  {
    myTrainHub1.init();
  }

  if (!myTrainHub2.isConnected())
  {
    myTrainHub2.init();
  }

  if (myRemote.isConnected() && myTrainHub1.isConnected() && myTrainHub2.isConnected() && !isInitialized)
  {
    Serial.println("System is initialized");
    isInitialized = true;
    delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
    // both activations are needed to get status updates
    myRemote.activatePortDevice(portLeft, remoteCallback);
    myRemote.activatePortDevice(portRight, remoteCallback);
    myRemote.setLedColor(WHITE);
  }

} // End of loop
