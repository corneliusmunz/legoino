/**
 * A Legoino example to control a train which has a motor connected
 * to the Port A of the Hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"
#include "PoweredUpHub.h"

// create a hub instance
PoweredUpHub myTrainHub1;
PoweredUpHub myTrainHub2;
PoweredUpRemote myRemote1;
PoweredUpRemote myRemote2;


PoweredUpHub::Port _port = PoweredUpHub::Port::A;


void setup() {
    Serial.begin(115200);
} 


// main loop
void loop() {

  if (!myTrainHub1.isConnected() && !myTrainHub1.isConnecting()) 
  {
    myTrainHub1.init(); // initalize the PoweredUpHub instance
    //myTrainHub.init("90:84:2b:03:19:7f"); //example of initializing an hub with a specific address
  }

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myTrainHub1.isConnecting()) {
    myTrainHub1.connectHub();
    if (myTrainHub1.isConnected()) {
      Serial.println("Connected to HUB1");
      myTrainHub2.init();
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }
  

  if (myTrainHub2.isConnecting()) {
    myTrainHub2.connectHub();
    if (myTrainHub2.isConnected()) {
      Serial.println("Connected to HUB2");
      myRemote1.init();
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  
  if (myRemote1.isConnecting()) {
    myRemote1.connectHub();
    if (myRemote1.isConnected()) {
      Serial.println("Connected to Remote1");
      myRemote2.init();
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  if (myRemote2.isConnecting()) {
    myRemote2.connectHub();
    if (myRemote2.isConnected()) {
      Serial.println("Connected to Remote2");
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }
  
} // End of loop
