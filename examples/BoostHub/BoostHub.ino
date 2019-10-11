/**
 * A BoostHub basic example to connect a boost hub, set the led color and the name of the hub and
 * do some basic movements on the boost map grid
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "BoostHub.h"

// create a hub instance
BoostHub myBoostHub;
BoostHub::Port _portC = BoostHub::Port::C;
BoostHub::Port _portD = BoostHub::Port::D;

void setup() {
    Serial.begin(115200);
    myBoostHub.init(); // initalize the BoostHub instance
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myBoostHub.isConnecting()) {
    myBoostHub.connectHub();
    if (myBoostHub.isConnected()) {
      Serial.println("Connected to HUB");
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myBoostHub.isConnected()) {

    char hubName[] = "myBoostHub";
    myBoostHub.setHubName(hubName);
    myBoostHub.setLedColor(GREEN);
    delay(1000);
    myBoostHub.setLedColor(RED);
    delay(1000);

    // lets do some movements on the boost map
    myBoostHub.moveForward(1);
    delay(2000);
    myBoostHub.rotateLeft(90);
    delay(2000);
    myBoostHub.moveForward(1);
    delay(2000);
    myBoostHub.rotateRight(90);
    delay(2000);
    myBoostHub.moveBack(1);
    delay(2000);
    myBoostHub.moveArcLeft(90);
    delay(2000);
    myBoostHub.moveArcRight(90);
    delay(2000);
    myBoostHub.setMotorSpeedForDegrees(_portC, 50, 1*360*2);
    delay(2000);
    myBoostHub.setMotorSpeedForDegrees(_portD, 50, 1*360*2);
    delay(2000);
    myBoostHub.shutDownHub();

  }
  
} // End of loop