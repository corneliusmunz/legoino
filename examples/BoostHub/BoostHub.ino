/**
 * A BoostHub basic example to connect a boost hub, set the led color and the name of the hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "BoostHub.h"

#define GREEN_LED_PIN 13
#define RED_LED_PIN 12

// create a hub instance
BoostHub myBoostHub;
bool isLedOn=false;

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
    myBoostHub.init(); // initalize the BoostHub instance
    myBoostHub.registerButtonCallback(buttonNotification);
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myBoostHub.isConnecting()) {
    myBoostHub.connectHub();
    if (myBoostHub.isConnected()) {
      Serial.println("Connected to HUB");
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myBoostHub.isConnected()) {

    char hubName[] = "myBoostHub";
    myBoostHub.setHubName(hubName);
  
    myBoostHub.setLedColor(GREEN);
    delay(1000);
    myBoostHub.setLedColor(RED);
    delay(1000);
    myBoostHub.moveForward(1);
    delay(2000);
    myBoostHub.rotateLeft(90);
    delay(2000);
    myBoostHub.moveForward(1);
    delay(2000);
    myBoostHub.rotateRight(90);
    delay(2000);
    myBoostHub.moveBack(1);
    delay(2000);
    myBoostHub.shutDownHub();

  }
  
} // End of loop
