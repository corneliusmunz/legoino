/**
 * A Boost basic example to connect a boost hub, set the led color and the name of the hub and
 * do some basic movements on the boost map grid
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Boost.h"

// create a hub instance
Boost myMoveHub;
byte portC = (byte)MoveHubPort::C;
byte portD = (byte)MoveHubPort::D;

void setup()
{
  Serial.begin(115200);
  myMoveHub.init(); // initalize the MoveHub instance
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
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myMoveHub.isConnected())
  {

    char hubName[] = "myMoveHub";
    myMoveHub.setHubName(hubName);
    myMoveHub.setLedColor(GREEN);
    delay(1000);
    myMoveHub.setLedColor(RED);
    delay(1000);

    // lets do some movements on the boost map
    myMoveHub.moveForward(1);
    delay(2000);
    myMoveHub.rotateLeft(90);
    delay(2000);
    myMoveHub.moveForward(1);
    delay(2000);
    myMoveHub.rotateRight(90);
    delay(2000);
    myMoveHub.moveBack(1);
    delay(2000);
    myMoveHub.moveArcLeft(90);
    delay(2000);
    myMoveHub.moveArcRight(90);
    delay(2000);
    myMoveHub.setTachoMotorSpeedForDegrees(portC, 50, 1 * 360 * 2);
    delay(2000);
    myMoveHub.shutDownHub();
  }

} // End of loop