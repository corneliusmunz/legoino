/**
 * A Train hub basic example to connect a PoweredUp hub, and set the speed of the train 
 * motor dependent on the detected color. If the train is stopped, you can start the train by
 * pressing the hub button.
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpHub.h"

// create a hub instance
PoweredUpHub myHub;
PoweredUpHub::Port _portA = PoweredUpHub::Port::A;
PoweredUpHub::Port _portB = PoweredUpHub::Port::B;


void setup() {
    Serial.begin(115200);
    myHub.init(); // initalize the PoweredUp hub instance
} 


// main loop
void loop() {

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myHub.isConnecting()) {
    myHub.connectHub();
    if (myHub.isConnected()) {
      Serial.println("Connected to HUB");
      // connect color/distance sensor to port c, activate sensor for updates
      myHub.activatePortDevice(_portB, 37);
      myHub.setLedColor(GREEN);
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can start your control-flow
  if (myHub.isConnected()) {
    
    delay(100);

    if (myHub.isButtonPressed()) {
        myHub.setMotorSpeed(_portA, 15);
    }
   
    // read color value of sensor
    int color = myHub.getColor();
    Serial.print("Color: ");
    Serial.println(color, DEC);

    // set hub LED color to detected color of sensor
    if (color == 9) { 
        myHub.setLedColor(RED);
        myHub.stopMotor(_portA);     
    } else if (color == 7){
        myHub.setLedColor(YELLOW);
        myHub.setMotorSpeed(_portA, 25);   
    }else if (color == 3){ 
        myHub.setLedColor(BLUE);
        myHub.setMotorSpeed(_portA, 35);
    } 

  }
  
} // End of loop
