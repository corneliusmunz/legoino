/**
 * A Legoino example for connecting multiple hubs at the same time
 * Two train hubs and to train remotes are connected
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "HubManager.h"
#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myTrainHub1;
// Lpf2Hub myTrainHub2;
Lpf2Hub myRemoteHub1;
// Lpf2Hub myRemote2;

Lpf2Hub *hub;

// create a hub manager instance
HubManager hubManager;

void setup()
{
  Serial.begin(115200);
  myTrainHub1.init("90:84:2b:03:19:7f", false);
  myTrainHub1._pServerAddress = new BLEAddress("90:84:2b:03:19:7f");
  // hubManager.AddHub(myTrainHub1, "90:84:2b:03:19:7f", "myTrainHub1");
  // hubManager.AddHub(myRemoteHub1, "78:0b:2c:43:4d:90", "myRemoteHub1");
  //hubManager.Start();
}

// main loop
void loop()
{

  delay(100);
  if (!myTrainHub1.isConnected())
  {
    myTrainHub1.connectHub();
    if (myTrainHub1.isConnected())
    {
      Serial.println("Connected to HUB");
      Serial.print("Hub address: ");
      Serial.println(myTrainHub1.getHubAddress().toString().c_str());
      Serial.print("Hub name: ");
      Serial.println(myTrainHub1.getHubName().c_str());
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  // delay(2000);
  // hub = hubManager.GetHubByName("myTrainHub1");
  // Serial.print("Expected hub by name: myTrainHub1 fetched hub: ");
  // Serial.println(hub->getHubName().c_str());

  // delay(2000);
  // hub = hubManager.GetHubByName("myRemoteHub1");
  // Serial.print("Expected hub by name: myRemoteHub1 fetched hub: ");
  // Serial.println(hub->getHubName().c_str());

  // delay(2000);
  // hub = hubManager.GetHubByAddress("90:84:2b:03:19:7f");
  // Serial.print("Expected hub by address: myTrainHub1 fetched hub: ");
  // Serial.println(hub->getHubName().c_str());

  // delay(2000);
  // hub = hubManager.GetHubByAddress("78:0b:2c:43:4d:90");
  // Serial.print("Expected hub by address: myRemoteHub1 fetched hub: ");
  // Serial.println(hub->getHubName().c_str());

  // delay(100);
  // hub = hubManager.GetHubByAddress("90:84:2b:03:19:7f");
  // Serial.print("isConnecting: ");
  // Serial.println(hub->isConnecting());

  // if (hub->isConnecting()) {
  //   Serial.println("isConnecting");
  //   hub->connectHub();
  //   if (hub->isConnected()) {
  //     Serial.println("Hub is connected");
  //   } else {
  //     Serial.println("Failed to connect to hub");
  //   }
  // }

} // End of loop