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
 * (c) Copyright 2019 - Cornelius Munz
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

void setup() {
    Serial.begin(115200);
    myRemote.init(); // initalize the remote instance and try to connect
} 


// main loop
void loop() {

  // connect flow
  if (myRemote.isConnecting()) {
    myRemote.connectHub();
    if (myRemote.isConnected()) {
      Serial.println("Connected to Remote");
      myRemote.setLedColor(GREEN);
      myHub.init(); // after connecting the remote, try to connect the hub
    } else {
      Serial.println("Failed to connect to Remote");
    }
  }

  if (myHub.isConnecting()) {
    myHub.connectHub();
    if (myHub.isConnected()) {
      Serial.println("Connected to Hub");
    } else {
      Serial.println("Failed to connect to Hub");
    }
  }

  if (myRemote.isConnected() && myHub.isConnected() && !isInitialized) {
     Serial.println("System is initialized");
      isInitialized = true;
      // both activations are needed to get status updates
      myRemote.activateButtonReports(); 
      myRemote.activatePortDevice(_portLeft, 55);
      myRemote.activatePortDevice(_portRight, 55);
      myRemote.setLedColor(WHITE);
      myHub.setLedColor(WHITE);
  }

  // if connected we can control the train motor on Port A with the remote
  if (isInitialized) {

    if (myRemote.isLeftRemoteUpButtonPressed() || myRemote.isRightRemoteUpButtonPressed()) {
      myRemote.setLedColor(GREEN);
      updatedSpeed = min(100, currentSpeed+10);
    } else if (myRemote.isLeftRemoteDownButtonPressed() || myRemote.isRightRemoteDownButtonPressed()) {
      myRemote.setLedColor(BLUE);
      updatedSpeed = max(-100, currentSpeed-10);
    } else if (myRemote.isLeftRemoteStopButtonPressed() || myRemote.isRightRemoteStopButtonPressed()) {
      myRemote.setLedColor(RED);
      updatedSpeed = 0;
    } else if (myRemote.isLeftRemoteButtonReleased() || myRemote.isRightRemoteButtonReleased()) {
      myRemote.setLedColor(WHITE);      
    }

    if (currentSpeed != updatedSpeed) {
      myHub.setMotorSpeed(_portA, updatedSpeed);
      currentSpeed = updatedSpeed;
    }

    Serial.print("Current speed:");
    Serial.println(currentSpeed, DEC);
    delay(100);

  }
  
} // End of loop
