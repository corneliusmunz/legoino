/**
 * A Legoino example to emulate a train hub which directly sends
 * out the motor commands as IR signals to a power function IR receiver. 
 * With this setup you can upgrade your power function systems to powerup systems
 * 
 * For the setup the IR LED has to be connected on the OUTPUT PIN 12 of the 
 * ESP controller. This will work out of the Box with the M5 Atom matrix/light
 * esp32 board which has a build in IR LED on Port 12
 * 
 * Port A of the powered up hub is mapped to the RED power function port
 * Port B of the powered up hub is mapped to the BLUE power function port
 * 
 * Example video: https://www.youtube.com/watch?v=RTNexxT4-yQ&t=16s
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2HubEmulation.h"
#include "LegoinoCommon.h"

// create a hub instance
Lpf2HubEmulation myEmulatedHub("BoostHub", HubType::BOOST_MOVE_HUB);

int32_t counter = 0;

void writeValueCallback(byte port, byte value)
{
  Serial.println("writeValueCallback: ");
  Serial.println(port, HEX);
  Serial.println(value, HEX);

  if (port == 0x00)
  {
    Serial.print("Hub Motor Port A received message: ");
    Serial.println(value, DEC);
  }

  if (port == 0x01)
  {
    Serial.print("Hub Motor Port B received message: ");
    Serial.println(value, DEC);
  }

  if (port == 0x32)
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
    delay(2000);
    myEmulatedHub.isPortInitialized = true;
    myEmulatedHub.attachDevice((byte)MoveHubPort::C, DeviceType::COLOR_DISTANCE_SENSOR);
    delay(1000);
    myEmulatedHub.attachDevice((byte)MoveHubPort::D, DeviceType::MEDIUM_LINEAR_MOTOR);
    delay(2000);
    myEmulatedHub.attachDevice((byte)MoveHubPort::LED, DeviceType::HUB_LED);
    delay(2000);
    myEmulatedHub.attachDevice((byte)MoveHubPort::TILT, DeviceType::MOVE_HUB_TILT_SENSOR);
    delay(2000);
    myEmulatedHub.isPortInitialized = true;
  }

  if (myEmulatedHub.isConnected && myEmulatedHub.isPortInitialized)
  {
    delay(1000);
    counter++;
    byte roll = 45 + (counter % 10);
    byte pitch = 50 - (counter % 10);
    byte yaw = (counter % 20) - 10;
    byte orientation = counter % 6;

    myEmulatedHub.updateColorDistanceSensor((byte)MoveHubPort::C, (byte)(counter % 10), (byte)(counter + 1) % 10, (byte)(counter + 2) % 10);
    myEmulatedHub.updateMotorSensor((byte)MoveHubPort::D, (byte)((counter % 255) - 127), counter);
    myEmulatedHub.updateMoveTiltSensor(roll, pitch, yaw, orientation, counter);
  }

} // End of loop