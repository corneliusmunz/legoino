/**
 * A Legoino example to connect to a Powered Up remote and a move hub to control the Vernie model
 * 
 * ATTENTION: The connection order is relevant!
 * 1) Power up the move Hub
 * 2) Power up the ESP32
 * 3) Power up the PoweredUp remote control
 * 
 * Just rotate the right remote stick that the buttons are aligned horizontally (+ button to the right)
 * 
 * Left Button UP     -> Vernie will move one step forward
 * Left Button DOWN   -> Vernie will move one step back
 * Right Button UP    -> Vernie will rotate to the right
 * Right Button DOWN  -> Vernie will rotate to the left
 * Right Stop Button  -> Vernie will fire the arrow
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"
#include "Boost.h"

// create a hub instance
Lpf2Hub myRemote;
Boost myHub;

byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
byte portD = (byte)MoveHubPort::D;

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

    if (portNumber == (byte)portLeft && buttonState == ButtonState::UP)
    {
      myHub.moveForward(1);
      Serial.println("MoveForward");
    }
    else if (portNumber == (byte)portLeft && buttonState == ButtonState::DOWN)
    {
      myHub.moveBack(1);
      Serial.println("MoveBack");
    }
    else if (portNumber == (byte)portRight && buttonState == ButtonState::UP)
    {
      myHub.rotateRight(30);
      Serial.println("RotateRight");
    }
    else if (portNumber == (byte)portRight && buttonState == ButtonState::DOWN)
    {
      myHub.rotateLeft(30);
      Serial.println("RotateLeft");
    }
    else if (portNumber == (byte)portRight && buttonState == ButtonState::STOP)
    {
      Serial.println("Fire...");
      myHub.setTachoMotorSpeedForDegrees(portD, -80, 20);
      delay(500);
      myHub.setTachoMotorSpeedForDegrees(portD, 100, 180);
      delay(800);
      myHub.setTachoMotorSpeedForDegrees(portD, -80, 120);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  myHub.init(); // initalize the remote instance and try to connect
}

// main loop
void loop()
{

  // connect flow
  if (myHub.isConnecting())
  {
    myHub.connectHub();
    if (myHub.isConnected())
    {
      Serial.println("Connected to HUB");
      myHub.setLedColor(GREEN);
      myRemote.init(); // after connecting the remote, try to connect the hub
    }
    else
    {
      Serial.println("Failed to connect to Remote");
    }
  }

  if (myRemote.isConnecting())
  {
    myRemote.connectHub();
    if (myRemote.isConnected())
    {
      Serial.println("Connected to Remote");
    }
    else
    {
      Serial.println("Failed to connect to Hub");
    }
  }

  if (myRemote.isConnected() && myHub.isConnected() && !isInitialized)
  {
    Serial.println("Is initialized");
    isInitialized = true;
    delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
    // both activations are needed to get status updates
    myRemote.activatePortDevice(portLeft, remoteCallback);
    myRemote.activatePortDevice(portRight, remoteCallback);
    myRemote.setLedColor(WHITE);
    myHub.setLedColor(WHITE);
  }

} // End of loop