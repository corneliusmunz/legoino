/**
 * A Legoino example to control a train which has a motor connected
 * to the Port A of the Hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Legoino.h"

// create a legoino instance
Legoino myTrainHub;

void setup() {
    Serial.begin(115200);
    myTrainHub.init(POWEREDUP); // initalize the legoino instance with the specific hubtype
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("We are now connected to Train HUB");
      char hubName[] = "myTrainHub";
      myTrainHub.setHubName(hubName);
    } else {
      Serial.println("We have failed to connect to the Train HUB");
    }
  }

  // if connected, you can set the actuators of the hub and read in values
  if (myTrainHub.isConnected()) {
  
    myTrainHub.setLedColor(GREEN);
    myTrainHub.setMotorSpeed(A, 25);
    delay(1000);

    myTrainHub.setLedColor(RED);
    myTrainHub.stopMotor(A);
    delay(1000);

    myTrainHub.setLedColor(BLUE);
    myTrainHub.setMotorSpeed(A, -25);
    delay(1000);

    myTrainHub.stopMotor(A);
    
    for (int idx=0; idx < 20; idx++) {
      myTrainHub.setLedRGBColor(random(255), random(255), random(255));
      delay(200);
    }

    myTrainHub.shutDownHub();
    delay(2000);

  }
  
} // End of loop
