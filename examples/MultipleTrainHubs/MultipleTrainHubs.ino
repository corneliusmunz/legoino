/**
 * A Legoino example for connecting multiple hubs at the same time
 * Two train hubs and to train remotes are connected
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myTrainHub1;
Lpf2Hub myTrainHub2;
Lpf2Hub myRemote1;
Lpf2Hub myRemote2;

void setup()
{
  Serial.begin(115200);
  myTrainHub1.init(); // initalize the PoweredUpHub instance
}

// main loop
void loop()
{
  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myTrainHub1.isConnecting())
  {
    myTrainHub1.connectHub();
    if (myTrainHub1.isConnected())
    {
      Serial.println("Connected to HUB1");
      myTrainHub2.init();
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  if (myTrainHub2.isConnecting())
  {
    myTrainHub2.connectHub();
    if (myTrainHub2.isConnected())
    {
      Serial.println("Connected to HUB2");
      myRemote1.init();
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  if (myRemote1.isConnecting())
  {
    myRemote1.connectHub();
    if (myRemote1.isConnected())
    {
      Serial.println("Connected to Remote1");
      myRemote2.init();
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  if (myRemote2.isConnecting())
  {
    myRemote2.connectHub();
    if (myRemote2.isConnected())
    {
      Serial.println("Connected to Remote2");
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  delay(100);

} // End of loop
