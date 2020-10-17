/**
 * A Legoino example to control a control plus hub
 * with a Motor connected on Port B using absolute positioning
 * 
 * (c) Copyright 2020 - Carl Alldis
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myHub;
byte portB = (byte)ControlPlusHubPort::B;

bool isCalibrated;

void setup() {
    Serial.begin(115200);
    isCalibrated = false;
} 


// main loop
void loop() {

  if (!myHub.isConnected() && !myHub.isConnecting()) 
  {
    myHub.init(); 
  }

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

    // if calibration is not yet complete, reset steering center (assumes motor is physically restricted at +/- 90 degrees)
    if (!isCalibrated) {
      myHub.setLedColor(ORANGE);
      // Move to the minimum endstop (i.e. -90)
      myHub.setTachoMotorSpeedForDegrees(portB, -100, 180);
      delay(2000);
      // Reset the encoded position to -90
      myHub.setAbsoluteMotorEncoderPosition(portB, -90);
      delay(1000);
      // Move to center
      myHub.setAbsoluteMotorPosition(portB, 100, 0);
      myHub.setLedColor(YELLOW);
      delay(5000);
      // Do not repeat calibration
      isCalibrated = true;
    }

    // Cycle between the min/max steering points, then back to center 
    myHub.setLedColor(GREEN);
    myHub.setAbsoluteMotorPosition(portB, 100, -90);
    delay(1000);
    myHub.setAbsoluteMotorPosition(portB, 100, -45);
    delay(1000);
    myHub.setAbsoluteMotorPosition(portB, 100, 45);
    delay(1000);
    myHub.setAbsoluteMotorPosition(portB, 100, 90);
    delay(1000);
    myHub.setAbsoluteMotorPosition(portB, 100, 0);
    delay(1000);
    myHub.setLedColor(RED);
    delay(1000);
  } else {
    Serial.println("ControlPlus hub is disconnected");
  }
  
} // End of loop