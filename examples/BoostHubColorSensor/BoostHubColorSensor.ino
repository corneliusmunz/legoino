/**
 * A BoostHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected color of the color/distance sensor
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
   
    // read color value of sensor
    int color = myBoostHub.getColor();

    // set hub LED color to detected color of sensor
    if (color == 3) {
      myBoostHub.setLedColor(BLUE);
    } else if (color == 6){
      myBoostHub.setLedColor(GREEN);
    }else if (color == 9){
      myBoostHub.setLedColor(RED);
    } else if (color == 10){
      myBoostHub.setLedColor(WHITE);
    } else {
      myBoostHub.setLedColor(NONE);
    }

  }
  
} // End of loop
