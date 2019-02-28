/**
 * A Lpf2Hub basic example to connect a hub, set the led color and the name of the hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

#define GREEN_LED_PIN 13
#define RED_LED_PIN 12

// create a lpf2hub instance
Lpf2Hub myHub;
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
    myHub.init(); // initalize the lpf2hub instance
    myHub.registerButtonCallback(buttonNotification);
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myHub.isConnecting()) {
    myHub.connectHub();
    if (myHub.isConnected()) {
      Serial.println("Connected to HUB");
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myHub.isConnected()) {

    char hubName[] = "myLpf2Hub";
    myHub.setHubName(hubName);
  
    myHub.setLedColor(GREEN);
    delay(1000);
    myHub.setLedColor(RED);
    delay(5000);
    myHub.shutDownHub();

  }
  
} // End of loop
