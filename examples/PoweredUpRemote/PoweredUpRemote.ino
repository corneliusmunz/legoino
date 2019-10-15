/**
 * A Legoino example to connect to a powered up hub and powered up remote
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"

// create a hub instance
PoweredUpRemote myRemote;

PoweredUpRemote::Port _portLeft = PoweredUpRemote::Port::LEFT;
PoweredUpRemote::Port _portRight = PoweredUpRemote::Port::RIGHT;


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
      myRemote.activateButtonReports();
      myRemote.setLedColor(GREEN);
    } else {
      Serial.println("Failed to connect to Remote");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myRemote.isConnected()) {

    delay(100);

  }
  
} // End of loop