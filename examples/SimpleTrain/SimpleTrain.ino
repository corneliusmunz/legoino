/**
 * A Legoino example to control a train which has a motor connected
 * to the Port A of the Hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Legoino.h"

#define GREEN_LED_PIN 13
#define RED_LED_PIN 12

// create a legoino instance
Legoino myTrainHub;
bool isLedOn=false;
Port _port=AB;

void buttonNotification(bool isPressed) {
   if (isPressed) {
     if (isLedOn) {
       digitalWrite(GREEN_LED_PIN, LOW);
       isLedOn=false;
     } else {
       digitalWrite(GREEN_LED_PIN, HIGH);
       isLedOn=true;
     }
   }
}

void setup() {
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    Serial.begin(115200);
    myTrainHub.init(); // initalize the legoino instance
    myTrainHub.registerButtonCallback(buttonNotification);
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("We are now connected to Train HUB");
      char hubName[] = "myBoostHub";
      myTrainHub.setHubName(hubName);
    } else {
      Serial.println("We failed to connect to the Train HUB");
    }
  }

  // if connected, you can set the actuators of the hub and read in values
  if (myTrainHub.isConnected()) {
  
    myTrainHub.setLedColor(GREEN);
    delay(1000);
    myTrainHub.setLedColor(RED);
    delay(1000);
    
    myTrainHub.setLedColor(BLUE);
    myTrainHub.setMotorSpeed(_port, -25);
    delay(1000);
    myTrainHub.stopMotor(_port);
    delay(1000);
    myTrainHub.setMotorSpeed(_port, 25);
    delay(1000);
    myTrainHub.stopMotor(_port);
    delay(1000);

  }
  
} // End of loop
