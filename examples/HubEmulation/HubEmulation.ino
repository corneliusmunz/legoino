/**
 * A Legoino example to control a train which has a motor connected
 * to the Port A of the Hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2HubEmulation.h"
#include <M5Atom.h>
#include <PowerFunctions.h>
#include "LegoinoCommon.h"

// create a hub instance
Lpf2HubEmulation myEmulatedHub("TrainHub", HubType::POWERED_UP_HUB);
// create a power functions instance
PowerFunctions pf(12, 0);

void setPfSpeed(byte port, byte value) {
    char speed = LegoinoCommon::MapSpeedReverse(value);
    Serial.print("speed: ");
    Serial.println(speed, DEC);
    if (speed > 0)
    {
      char pfSpeed = (char)(speed / 14.0);
      Serial.print("speed forward: ");
      Serial.println(pfSpeed, DEC);
      pf.single_pwm(port, pfSpeed);
    }
    if (speed == 0)
    {
      Serial.println("stopp");
      pf.single_pwm(port, PWM_BRK);
    }

    if (speed < 0)
    {
      char pfSpeed = 15 - (char)(-speed / 14.0);
      Serial.print("speed reverse: ");
      Serial.println(pfSpeed, DEC);
      pf.single_pwm(port, pfSpeed);
    }
}

void writeValueCallback(byte port, byte value)
{
  Serial.println("writeValueCallback: ");
  Serial.println(port, HEX);
  Serial.println(value, HEX);

  if (port == 0x00)
  {
    setPfSpeed(0x00, value);
  }

  if (port == 0x01)
  {
    setPfSpeed(0x01, value);
  }

  if (port == 0x32)
  {
    CRGB color = 0x000000;
    switch (value)
    {
    case 0: 
      color = 0x000000;
      break;
    case 1: 
      color = 0x000000;
      break;
    case 2: 
      color = 0xFF00FF;
      break;
    case 3: 
      color = 0x0000FF;
      break;
    case 4: 
      color = 0x00ffff;
      break;
    case 5: 
      color = 0x00cc99;
      break;
    case 6: 
      color = 0x00cc00;
      break;
    case 7: 
      color = 0xffff00;
      break;
    case 8: 
      color = 0xff6600;
      break;
    case 9: 
      color = 0xFF0000;
      break;
    case 10: 
      color = 0xFFFFFF;
      break;
    }
    M5.dis.fillpix(color);
  }
}

void setup()
{
  Serial.begin(115200);

  M5.begin(true, false, true);
  delay(50);
  M5.dis.clear();

  myEmulatedHub.setWritePortCallback(&writeValueCallback);
  myEmulatedHub.start();
}

// main loop
void loop()
{

  if (myEmulatedHub.isConnected && !myEmulatedHub.isPortInitialized)
  {
    delay(1000);
    myEmulatedHub.isPortInitialized = true;
    myEmulatedHub.attachDevice(0x00, DeviceType::MEDIUM_LINEAR_MOTOR);
    delay(1000);
    myEmulatedHub.attachDevice(0x32, DeviceType::HUB_LED);
    delay(1000);
    myEmulatedHub.attachDevice(0x01, DeviceType::MEDIUM_LINEAR_MOTOR);
    delay(1000);
    myEmulatedHub.attachDevice(0x03, DeviceType::COLOR_DISTANCE_SENSOR);
    delay(1000);
    //M5.dis.fillpix(CRGB::Green);
  }

} // End of loop