/**
 * A Legoino example to emulate a MoveHub (Boost) which reports 
 * sensor values (color, distance, tilt, motor position, motor speed) of "virtually"
 * connected sensors on the ports
 * 
 * (c) Copyright 2021 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2HubEmulation.h"
#include "LegoinoCommon.h"

// create a hub instance
Lpf2HubEmulation myEmulatedHub("TrainHub", HubType::POWERED_UP_HUB);

int32_t counter = 0;

void writeValueCallback(byte port, byte value)
{
  Serial.println("writeValueCallback: ");
  Serial.println(port, HEX);
  Serial.println(value, HEX);

  if (port == (byte)PoweredUpHubPort::LED)
  {
    Serial.print("Hub LED command received with color: ");
    Serial.println(LegoinoCommon::ColorStringFromColor(value).c_str());
  }
}

void setup()
{
  Serial.begin(115200);
  // define the callback function if a write message event on the characteristic occurs
  myEmulatedHub.setWritePortCallback(&writeValueCallback);
  myEmulatedHub.start();
}

// main loop
void loop()
{

  // if an app is connected, attach some devices on the ports to signalize
  // the app that values could be received/written to that ports
  if (myEmulatedHub.isConnected && !myEmulatedHub.isPortInitialized)
  {
    delay(3000);
    myEmulatedHub.attachDevice((byte)PoweredUpHubPort::A, DeviceType::COLOR_DISTANCE_SENSOR);
    delay(2000);
    myEmulatedHub.attachDevice((byte)PoweredUpHubPort::B, DeviceType::MEDIUM_LINEAR_MOTOR);
    delay(2000);
    myEmulatedHub.attachDevice((byte)PoweredUpHubPort::LED, DeviceType::HUB_LED);
    delay(2000);
    // myEmulatedHub.attachDevice((byte)MoveHubPort::TILT, DeviceType::MOVE_HUB_TILT_SENSOR);
    // delay(2000);
    // myEmulatedHub.attachDevice((byte)ControlPlusHubPort::GYRO, DeviceType::TECHNIC_MEDIUM_HUB_GYRO_SENSOR);
    // delay(2000);
    //myEmulatedHub.attachDevice((byte)ControlPlusHubPort::TILT, DeviceType::TECHNIC_MEDIUM_HUB_TILT_SENSOR);
    // delay(2000);
    myEmulatedHub.isPortInitialized = true;
  }

  if (myEmulatedHub.isConnected && myEmulatedHub.isPortInitialized)
  {
    delay(1000);
    counter++;

    // create test sensor values based on increasing counter
    byte roll = 45 + (counter % 10);
    byte pitch = 50 - (counter % 10);
    byte yaw = (counter % 20) - 10;
    byte orientation = counter % 6;
    byte color = (byte)(counter % 10);
    byte distance = (byte)(counter + 1) % 10;
    byte reflectivness = (byte)(counter + 2) % 10;
    byte speed = (byte)((counter % 255) - 127);
    byte position = counter - 100;

    // update sensor values
    myEmulatedHub.updateColorDistanceSensor((byte)PoweredUpHubPort::A, color, distance, reflectivness);
    myEmulatedHub.updateMotorSensor((byte)PoweredUpHubPort::B, speed, position);
    // myEmulatedHub.updateMoveHubTiltSensor(roll, pitch, yaw, orientation, counter);
    //myEmulatedHub.updateTechnicHubTiltSensor(roll, pitch, yaw);
  }

} // End of loop