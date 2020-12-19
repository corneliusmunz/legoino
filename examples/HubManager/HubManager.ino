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
Lpf2Hub myRemote1;
// Lpf2Hub myRemote2;

// create a hub manager instance
HubManager hubManager;

void setup()
{
  Serial.begin(115200);
  hubManager.AddHub(myTrainHub1, "90:84:2b:03:19:7f");
  // hubManager.AddHub(myTrainHub2);
  //hubManager.AddHub(myRemote1, "78:0b:2c:43:4d:90");
  // hubManager.AddHub(myRemote2);
  hubManager.StartDiscovery();
}

// main loop
void loop()
{
  delay(100);

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (hubManager.IsConnectionFinished())
  {
    Serial.println("System is ready...");

    for (int i = 0; i < hubManager.ManagedHubs.size(); i++)
    {
      Serial.println((*hubManager.ManagedHubs[i]).getHubName().c_str());
    }
  }
  else
  {
    Serial.println("Initializing system...");
  }

} // End of loop
