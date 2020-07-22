/**
 * A Legoino example to connect two train hubs and one powered up remote. 
 * 
 * 1) Power up the ESP32
 * 2) Power up the Remote and the Train Hubs in any order
 * 
 * You can change the motor speed of train 1 with the left (A) remote buttons
 * You can change the motor speed of train 2 with the right (B) remote buttons
 * 
 * (c) Copyright 2020
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"
#include "PoweredUpHub.h"

// create a hub instance
PoweredUpRemote myRemote;
PoweredUpHub myTrainHub1;
PoweredUpHub myTrainHub2;

PoweredUpRemote::Port _portLeft = PoweredUpRemote::Port::LEFT;
PoweredUpRemote::Port _portRight = PoweredUpRemote::Port::RIGHT;
PoweredUpHub::Port _portA = PoweredUpHub::Port::A;

int currentSpeedTrain1 = 0;
int currentSpeedTrain2 = 0;
int updatedSpeedTrain1 = 0;
int updatedSpeedTrain2 = 0;

bool isInitialized = false;

void setup() {
    Serial.begin(115200);
    myRemote.init(); // initialize the remote control hub
    myTrainHub1.init("90:84:2b:03:19:7f"); // initialize the listening train hub 1 // here you have to use your own device ids
    myTrainHub2.init("90:84:2b:06:76:a6"); // initialize the listening train hub 2 // here you have to use your own device ids
} 


// main loop
void loop() {

  //wait for three elements

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

  if (myTrainHub1.isConnecting())
  {
    if (myTrainHub1.getHubType() == POWERED_UP_HUB)
    {
      myTrainHub1.connectHub();
      myTrainHub1.setLedColor(BLUE);
      Serial.println("train hub 1 connected.");
    }
  }

  if (myTrainHub2.isConnecting())
  {
    if (myTrainHub2.getHubType() == POWERED_UP_HUB)
    {
      myTrainHub2.connectHub();
      myTrainHub2.setLedColor(YELLOW);
      Serial.println("train hub 2 connected.");
    }
  }

  if (!myRemote.isConnected())
  {
    myRemote.init();
  }

  if (!myTrainHub1.isConnected())
  {
    myTrainHub1.init();
  }

  if (!myTrainHub2.isConnected())
  {
    myTrainHub2.init();
  }
  
  if (myRemote.isConnected() && myTrainHub1.isConnected() && myTrainHub2.isConnected() && !isInitialized) {
     Serial.println("System is initialized");
      isInitialized = true;
      // both activations are needed to get status updates
      myRemote.activateButtonReports(); 
      myRemote.activatePortDevice(_portLeft, 55);
      myRemote.activatePortDevice(_portRight, 55);
      myRemote.setLedColor(WHITE);
  }

  // if connected we can control the train motor of hub 1 with the left buttons and the train motor of hub 2 with the right buttons. Both motors should be connected on Port Ae
  if (isInitialized) {

    // Do the logic for left buttons of remote control and Train Hub 1
    if (myRemote.isLeftRemoteUpButtonPressed()) {
      updatedSpeedTrain1 = min(100, currentSpeedTrain1+10);
    } else if (myRemote.isLeftRemoteDownButtonPressed()) {
      updatedSpeedTrain1 = min(100, currentSpeedTrain1-10);
    } else if (myRemote.isLeftRemoteStopButtonPressed()) {
      updatedSpeedTrain1 = 0;
    } 

    if (currentSpeedTrain1 != updatedSpeedTrain1) {
      myTrainHub1.setMotorSpeed(_portA, updatedSpeedTrain1);
      currentSpeedTrain1 = updatedSpeedTrain1;
    }

    Serial.print("Current speed train 1:");
    Serial.println(currentSpeedTrain1, DEC);

    // Do the logic for right buttons of remote control and Train Hub 2
    if (myRemote.isRightRemoteUpButtonPressed()) {
      updatedSpeedTrain2 = min(100, currentSpeedTrain2+10);
    } else if (myRemote.isRightRemoteDownButtonPressed()) {
      updatedSpeedTrain2 = min(100, currentSpeedTrain2-10);
    } else if (myRemote.isRightRemoteStopButtonPressed()) {
      updatedSpeedTrain2 = 0;
    } 

    if (currentSpeedTrain2 != updatedSpeedTrain2) {
      myTrainHub2.setMotorSpeed(_portA, updatedSpeedTrain2);
      currentSpeedTrain2 = updatedSpeedTrain2;
    }

    Serial.print("Current speed train 2:");
    Serial.println(currentSpeedTrain2, DEC);

    
    delay(100);

  }
  
} // End of loop
