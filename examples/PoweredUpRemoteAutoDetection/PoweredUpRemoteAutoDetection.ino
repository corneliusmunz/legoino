/**
 * A Legoino example to connect to a Powered Up remote and a train hub. 
 * 
 * 1) Power up the ESP32
 * 2) Power up the Remote and the Train Hub in any order
 * 
 * You can change the motor speed with the left (A) remote buttons
 * 
 * (c) Copyright 2019 - Nicolas HILAIRE
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"
#include "PoweredUpHub.h"

// create a hub instance
PoweredUpRemote myRemote;
PoweredUpHub myHub;

PoweredUpRemote::Port _portLeft = PoweredUpRemote::Port::LEFT;
PoweredUpRemote::Port _portRight = PoweredUpRemote::Port::RIGHT;
PoweredUpHub::Port _portA = PoweredUpHub::Port::A;

int currentSpeed = 0;
int updatedSpeed = 0;
bool isInitialized = false;

void setup() {
    Serial.begin(115200);
    myRemote.init(); // initialize the remote hub
    myHub.init(); // initialize the listening hub
} 


// main loop
void loop() {

  //wait for two elements

  if (myRemote.isConnecting())
  {
    if (myRemote.getHubType() == POWERED_UP_REMOTE)
    {
      //This is the right device 
      if (!myRemote.connectHub())
      {
        Serial.println("Unable to connect to hub");
      }
      else
      {
        myRemote.setLedColor(GREEN);
        Serial.println("Remote connected.");
      }
      
    }
  }

  if (myHub.isConnecting())
  {
    if (myHub.getHubType() == POWERED_UP_HUB)
    {
      myHub.connectHub();
      myHub.setLedColor(GREEN);
      Serial.println("powered up hub connected.");
    }
  }

  if (!myRemote.isConnected())
  {
    myRemote.init();
  }

  if (! myHub.isConnected())
  {
    myHub.init();
  }
  
  if (myRemote.isConnected() && myHub.isConnected() && !isInitialized) {
     Serial.println("System is initialized");
      isInitialized = true;
      // both activations are needed to get status updates
      myRemote.activateButtonReports(); 
      myRemote.activatePortDevice(_portLeft, 55);
      myRemote.activatePortDevice(_portRight, 55);
      myRemote.setLedColor(WHITE);
      myHub.setLedColor(WHITE);
  }

  // if connected we can control the train motor on Port A with the remote
  if (isInitialized) {

    if (myRemote.isLeftRemoteUpButtonPressed() || myRemote.isRightRemoteUpButtonPressed()) {
      myRemote.setLedColor(GREEN);
      updatedSpeed = min(100, currentSpeed+10);
    } else if (myRemote.isLeftRemoteDownButtonPressed() || myRemote.isRightRemoteDownButtonPressed()) {
      myRemote.setLedColor(BLUE);
      updatedSpeed = min(100, currentSpeed-10);
    } else if (myRemote.isLeftRemoteStopButtonPressed() || myRemote.isRightRemoteStopButtonPressed()) {
      myRemote.setLedColor(RED);
      updatedSpeed = 0;
    } else if (myRemote.isLeftRemoteButtonReleased() || myRemote.isRightRemoteButtonReleased()) {
      myRemote.setLedColor(WHITE);      
    }

    if (currentSpeed != updatedSpeed) {
      myHub.setMotorSpeed(_portA, updatedSpeed);
      currentSpeed = updatedSpeed;
    }

    Serial.print("Current speed:");
    Serial.println(currentSpeed, DEC);
    delay(100);

  }
  
} // End of loop