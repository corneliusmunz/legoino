/**
 * A BoostHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected rotation of the boost tacho motor
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
      // connect color/distance sensor to port c, activate sensor for updates
      myBoostHub.activatePortDevice(_portC, 37);
      // connect boost tacho motor  to port d, activate sensor for updates
      myBoostHub.activatePortDevice(_portD, 38);
      myBoostHub.setLedColor(GREEN);
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myBoostHub.isConnected()) {
    
    delay(100);
   
    // read rotation value in degrees of the boost tacho motor
    int rotation = myBoostHub.getTachoMotorRotation();

    // set hub LED color dependent on the absolute angle of the rotation (mapping from angle to rainbow color)
    myBoostHub.setLedHSVColor(abs(rotation), 1.0, 1.0);

  }
  
} // End of loop
