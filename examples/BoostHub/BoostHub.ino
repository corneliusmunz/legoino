/**
 * A BoostHub basic example to connect a boost hub, set the led color and the name of the hub and
 * do some basic movements on the boost map grid
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
      myBoostHub.activatePortDevice(_portC, 37);
      myBoostHub.activatePortDevice(_portD, 38);
      myBoostHub.setLedColor(GREEN);
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myBoostHub.isConnected()) {

    //char hubName[] = "myBoostHub";
    //myBoostHub.setHubName(hubName);
    
    delay(100);

    // int rotation = myBoostHub.getRotation();
    // myBoostHub.setLedHSVColor(abs(rotation), 1.0, 1.0);

    // int rotation = myBoostHub.getRotation()%360;
    // if (rotation >= 0 && rotation < 30) {
    //   myBoostHub.setLedColor(PINK);
    // } else if (rotation >= 30 && rotation < 30){
    //   myBoostHub.setLedColor(PURPLE);
    // }else if (rotation >= 60 && rotation < 90){
    //   myBoostHub.setLedColor(BLUE);
    // }else if (rotation >= 90 && rotation < 120){
    //   myBoostHub.setLedColor(LIGHTBLUE);
    // }else if (rotation >= 120 && rotation < 150){
    //   myBoostHub.setLedColor(GREEN);
    // }else if (rotation >= 150 && rotation < 180){
    //   myBoostHub.setLedColor(ORANGE);
    // }else if (rotation >= 180 && rotation < 210){
    //   myBoostHub.setLedColor(RED);
    // }else if (rotation >= 210 && rotation < 240){
    //   myBoostHub.setLedColor(WHITE);
    // }
    
    int color = myBoostHub.getColor();
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

    

    // lets do some movements on the boost map
    //myBoostHub.moveForward(1);
    //delay(2000);

    //delay(1000);
    //myBoostHub.moveBack(1);
    //delay(1000);
    //myBoostHub.deactivatePortDevice(0x03, 37);
    //delay(1000);
    /*
    myBoostHub.rotateLeft(90);
    delay(2000);
    myBoostHub.moveForward(1);
    delay(2000);
    myBoostHub.rotateRight(90);
    delay(2000);
    myBoostHub.moveBack(1);
    delay(2000);
    myBoostHub.moveArcLeft(90);
    delay(2000);
    myBoostHub.moveArcRight(90);
    delay(2000);
    myBoostHub.setMotorSpeedForDegrees(_portC, 50, 1*360*2);
    delay(2000);
    myBoostHub.setMotorSpeedForDegrees(_portD, 50, 1*360*2);
    delay(2000);
    myBoostHub.shutDownHub();
    */

  }
  
} // End of loop
