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
} 


// main loop
void loop() {

  if (!myTrainHub.isConnected() && !myTrainHub.isConnecting()) 
  {
    myTrainHub.init(); // initalize the PoweredUpHub instance
    //myTrainHub.init("90:84:2b:03:19:7f"); //example of initializing an hub with a specific address
  }

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("Connected to HUB");
      Serial.print("Hub address: ");
      Serial.println(myTrainHub.getHubAddress().toString().c_str());
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

  } else {
    Serial.println("Train hub is disconnected");
  }
  
} // End of loop
