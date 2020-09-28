/**
 * A BoostHub basic example to connect a boost hub and try to get the
 * hub device infos like battery level, Rssi and firmware version
 * 
 * (c) Copyright 2020 - Cornelius Munz
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
      // The activation of hub updates is directly done in the connection procedure of the library
      // The activation of the Tilt sensor has to be done "manually" with the following command
      // after activating the port device, values will be received if changes occur
      myBoostHub.activatePortDevice(0x3A, 40); // Tilt-Sensor
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, print out continously the hub property values
  if (myBoostHub.isConnected()) {

    delay(1000);

    Serial.print("Rssi: ");
    Serial.println(myBoostHub.getRssi(), DEC);

    Serial.print("BatteryLevel [%]: ");
    Serial.println(myBoostHub.getBatteryLevel(), DEC);

    Serial.print("Current [mA]: ");
    Serial.println(myBoostHub.getHubCurrent(), DEC);

    Serial.print("Voltage [V]: ");
    Serial.println(myBoostHub.getHubVoltage(), DEC);

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