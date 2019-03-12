/**
 * A Legoino example to control a train which has a motor connected
 * to the Port A of the Hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpHub.h"

// create a hub instance
PoweredUpHub myTrainHub;
PoweredUpHub::Port _port = PoweredUpHub::Port::A;

void setup() {
    Serial.begin(115200);
    myTrainHub.init(); // initalize the PoweredUpHub instance
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("Connected to HUB");
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myTrainHub.isConnected()) {

    char hubName[] = "myTrainHub";
    myTrainHub.setHubName(hubName);
  
    myTrainHub.setLedColor(GREEN);
    delay(1000);
    myTrainHub.setLedColor(RED);
    delay(1000);
    myTrainHub.setMotorSpeed(_port, 35);
    delay(1000);
    myTrainHub.stopMotor(_port);
    delay(1000);
    myTrainHub.setMotorSpeed(_port, -35);
    delay(1000);
    myTrainHub.stopMotor(_port);
    delay(1000);    
    myTrainHub.shutDownHub();

  }
  
} // End of loop
