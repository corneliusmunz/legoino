/**
 * A Legoino example to connect to a Powered Up remote. If the left buttons are pressed,
 * the color of the Powered Up remote LED will change
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"

// create a hub instance
PoweredUpRemote myRemote;

PoweredUpRemote::Port _portLeft = PoweredUpRemote::Port::LEFT;


void setup() {
    Serial.begin(115200);
    myRemote.init(); // initalize the PoweredUpHub instance
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myRemote.isConnecting()) {
    myRemote.connectHub();
    if (myRemote.isConnected()) {
      Serial.println("Connected to Remote");
      // both activations are needed to get status updates
      myRemote.activateButtonReports(); 
      myRemote.activatePortDevice(_portLeft, 55);
      myRemote.setLedColor(WHITE);
    } else {
      Serial.println("Failed to connect to Remote");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myRemote.isConnected()) {

    delay(100);
    if (myRemote.isLeftRemoteUpButtonPressed()) {
      myRemote.setLedColor(GREEN);
    } else if (myRemote.isLeftRemoteDownButtonPressed()) {
      myRemote.setLedColor(BLUE);
    } else if (myRemote.isLeftRemoteStopButtonPressed()) {
      myRemote.setLedColor(RED);
    } else if (myRemote.isLeftRemoteButtonReleased()) {
      myRemote.setLedColor(WHITE);      
    }

  }
  
} // End of loop