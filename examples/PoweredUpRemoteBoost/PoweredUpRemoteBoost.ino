/**
 * A Legoino example to connect to a Powered Up remote and a boost hub to control the Vernie model
 * 
 * ATTENTION: The connection order is relevant!
 * 1) Power up the Boost Hub
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
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"
#include "BoostHub.h"

// create a hub instance
PoweredUpRemote myRemote;
BoostHub myHub;

PoweredUpRemote::Port _portLeft = PoweredUpRemote::Port::LEFT;
PoweredUpRemote::Port _portRight = PoweredUpRemote::Port::RIGHT;
BoostHub::Port _portD = BoostHub::Port::D;

bool isInitialized = false;

void setup() {
    Serial.begin(115200);
    myHub.init(); // initalize the remote instance and try to connect
} 


// main loop
void loop() {

  // connect flow
  if (myHub.isConnecting()) {
    myHub.connectHub();
    if (myHub.isConnected()) {
      Serial.println("Connected to HUB");
      myHub.setLedColor(GREEN);
      myRemote.init(); // after connecting the remote, try to connect the hub
    } else {
      Serial.println("Failed to connect to Remote");
    }
  }

  if (myRemote.isConnecting()) {
    myRemote.connectHub();
    if (myRemote.isConnected()) {
      Serial.println("Connected to Remote");
    } else {
      Serial.println("Failed to connect to Hub");
    }
  }

  if (myRemote.isConnected() && myHub.isConnected() && !isInitialized) {
      Serial.println("Is initialized");
      isInitialized = true;
      // both activations are needed to get status updates
      myRemote.activateButtonReports(); 
      myRemote.activatePortDevice(_portLeft, 55);
      myRemote.activatePortDevice(_portRight, 55);
      myRemote.setLedColor(WHITE);
      myHub.setLedColor(WHITE);
  }

  // if connected we can control vernie with the remote
  if (isInitialized) {

    if (myRemote.isLeftRemoteUpButtonPressed()) {
      myHub.moveForward(1);
      Serial.println("MoveForward");
    } else if (myRemote.isLeftRemoteDownButtonPressed()) {
      myHub.moveBack(1);
      Serial.println("MoveBack");
    } else if (myRemote.isRightRemoteUpButtonPressed()) {
      myHub.rotateRight(30);
      Serial.println("RotateRight");
    } else if (myRemote.isRightRemoteDownButtonPressed()) {
      myHub.rotateLeft(30);
      Serial.println("RotateLeft");
    } else if (myRemote.isRightRemoteStopButtonPressed()) {
      Serial.println("Fire...");
      myHub.setMotorSpeedForDegrees(_portD, -80, 20);
      delay(500);
      myHub.setMotorSpeedForDegrees(_portD, 100, 180);
      delay(800);
      myHub.setMotorSpeedForDegrees(_portD, -80, 120);
    }


    delay(100);

  }
  
} // End of loop