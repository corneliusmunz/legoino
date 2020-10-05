/**
 * A BoostHub basic example to connect a boost hub and try to get the
 * hub device infos like battery level, Rssi and firmware version. Additionally the read out
 * of the hub button with a callback is shown.
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "BoostHub.h"

// create a hub instance
BoostHub myBoostHub;

void hubPropertyChangeCallback(HubPropertyReference hubProperty, uint8_t *pData)
{
  Serial.print("HubProperty: ");
  Serial.println((byte)hubProperty, HEX);

  if (hubProperty == HubPropertyReference::RSSI)
  {
    Serial.print("RSSI: ");
    Serial.println(myBoostHub.parseRssi(pData), DEC);
    return;
  }

  if (hubProperty == HubPropertyReference::ADVERTISING_NAME)
  {
    Serial.print("Advertising Name: ");
    Serial.println(myBoostHub.parseHubAdvertisingName(pData).c_str());
    return;
  }

  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE)
  {
    Serial.print("BatteryLevel: ");
    Serial.println(myBoostHub.parseBatteryLevel(pData), DEC);
    return;
  }

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    Serial.print("Button: ");
    Serial.println((byte)myBoostHub.parseHubButton(pData), HEX);
    return;
  }

  if (hubProperty == HubPropertyReference::BATTERY_TYPE)
  {
    Serial.print("BatteryType: ");
    Serial.println(myBoostHub.parseBatteryType(pData), HEX);
    return;
  }

  if (hubProperty == HubPropertyReference::FW_VERSION)
  {
    Version version = myBoostHub.parseVersion(pData);
    Serial.print("FWVersion: ");
    Serial.print(version.Major);
    Serial.print("-");
    Serial.print(version.Minor);
    Serial.print("-");
    Serial.print(version.Bugfix);
    Serial.print(" Build: ");
    Serial.println(version.Build);

    return;
  }

  if (hubProperty == HubPropertyReference::HW_VERSION)
  {
    Version version = myBoostHub.parseVersion(pData);
    Serial.print("HWVersion: ");
    Serial.print(version.Major);
    Serial.print("-");
    Serial.print(version.Minor);
    Serial.print("-");
    Serial.print(version.Bugfix);
    Serial.print(" Build: ");
    Serial.println(version.Build);

    return;
  }
}

// callback function to handle updates of sensor values
void portValueChangeCallback(byte portNumber, DeviceType deviceType, uint8_t *pData)
{

  if (deviceType == DeviceType::VOLTAGE_SENSOR)
  {
    double voltage = myBoostHub.parseVoltageSensor(pData);
    Serial.print("Voltage: ");
    Serial.println(voltage, 2);
    return;
  }

  if (deviceType == DeviceType::CURRENT_SENSOR)
  {
    double current = myBoostHub.parseCurrentSensor(pData);
    Serial.print("Current: ");
    Serial.println(current, 2);
    return;
  }

  if (deviceType == DeviceType::MOVE_HUB_TILT_SENSOR)
  {
    int x = myBoostHub.parseBoostTiltSensorX(pData);
    int y = myBoostHub.parseBoostTiltSensorY(pData);
    Serial.print("Tilt X: ");
    Serial.print(x, DEC);
    Serial.print(" Y: ");
    Serial.println(y, DEC);
  }
}

void setup()
{
  Serial.begin(115200);
  myBoostHub.init(); // initalize the BoostHub instance
}

// main loop
void loop()
{

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myBoostHub.isConnecting())
  {
    myBoostHub.connectHub();
    if (myBoostHub.isConnected())
    {
      Serial.println("Connected to HUB");
      delay(50); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
      myBoostHub.activateHubPropertyUpdate(HubPropertyReference::FW_VERSION, hubPropertyChangeCallback);
      delay(50);
      myBoostHub.activateHubPropertyUpdate(HubPropertyReference::HW_VERSION, hubPropertyChangeCallback);
      delay(50);
      myBoostHub.activateHubPropertyUpdate(HubPropertyReference::ADVERTISING_NAME, hubPropertyChangeCallback);
      delay(50);
      myBoostHub.activateHubPropertyUpdate(HubPropertyReference::BATTERY_TYPE, hubPropertyChangeCallback);
      delay(50);
      myBoostHub.activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubPropertyChangeCallback);
      delay(50);
      myBoostHub.activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubPropertyChangeCallback);
      delay(50);
      myBoostHub.activateHubPropertyUpdate(HubPropertyReference::RSSI, hubPropertyChangeCallback);

      delay(50);
      myBoostHub.activatePortDevice(BoostHub::Port::TILT, portValueChangeCallback);
      delay(50);
      myBoostHub.activatePortDevice(BoostHub::Port::CURRENT, portValueChangeCallback);
      delay(50);
      myBoostHub.activatePortDevice(BoostHub::Port::VOLTAGE, portValueChangeCallback);
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, print out continously the hub property values

} // End of loop