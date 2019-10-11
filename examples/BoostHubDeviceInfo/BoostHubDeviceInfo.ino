/**
 * A BoostHub basic example to connect a boost hub and try to get the
 * hub device infos like battery level, Rssi and firmwar version
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "BoostHub.h"

// create a hub instance
BoostHub myBoostHub;

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
      myBoostHub.activatePortDevice(0x3A, 40); // Tilt-Sensor
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myBoostHub.isConnected()) {

    delay(1000);

    Serial.print("Rssi: ");
    Serial.println(myBoostHub.getRssi(), DEC);

    Serial.print("BatteryLevel [%]: ");
    Serial.println(myBoostHub.getBatteryLevel(), DEC);

    Serial.print("Firmware Version: ");
    Serial.print(myBoostHub.getFirmwareVersionMajor(), DEC);
    Serial.print("-");
    Serial.print(myBoostHub.getFirmwareVersionMinor(), DEC);
    Serial.print("-");
    Serial.print(myBoostHub.getFirmwareVersionBugfix(), DEC);
    Serial.print("-");
    Serial.println(myBoostHub.getFirmwareVersionBuild(), DEC);

    Serial.print("Hardware Version: ");
    Serial.print(myBoostHub.getHardwareVersionMajor(), DEC);
    Serial.print("-");
    Serial.print(myBoostHub.getHardwareVersionMinor(), DEC);
    Serial.print("-");
    Serial.print(myBoostHub.getHardwareVersionBugfix(), DEC);
    Serial.print("-");
    Serial.println(myBoostHub.getHardwareVersionBuild(), DEC);

    Serial.print("Tilt [x/y]: ");
    Serial.print(myBoostHub.getTiltX(), DEC);
    Serial.print("/");
    Serial.println(myBoostHub.getTiltY(), DEC);

  }
  
} // End of loop