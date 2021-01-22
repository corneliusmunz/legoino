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
Lpf2HubEmulation myEmulatedHub("Train1Hub", HubType::POWERED_UP_HUB);

int32_t position = 0;

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
    myEmulatedHub.attachDevice((byte)PoweredUpHubPort::A, DeviceType::MEDIUM_LINEAR_MOTOR);
    delay(2000);
    myEmulatedHub.attachDevice((byte)PoweredUpHubPort::LED, DeviceType::HUB_LED);
    // delay(2000);
    // myEmulatedHub.attachDevice((byte)PoweredUpHubPort::B, DeviceType::TRAIN_MOTOR);
    delay(2000);
    myEmulatedHub.isPortInitialized = true;
  }

  // if (!myEmulatedHub.isConnected && myEmulatedHub.isPortInitialized)
  // {
  //   myEmulatedHub.isPortInitialized = false;
  // }

  if (myEmulatedHub.isConnected && myEmulatedHub.isPortInitialized)
  {
    delay(250);
    position ++;
    Serial.print("position: ");
    Serial.println(position, DEC);

    byte speed = -127 + (byte)(position % 255);
    char *positionBytes = static_cast<char *>(static_cast<void *>(&position));

    // combined mode
    std::string payload;
    payload.push_back((char)PoweredUpHubPort::A);
    payload.push_back((char)0x00); //0000
    payload.push_back((char)0x03); // 0011 (first mode 2 second mode 1)
    payload.push_back((char)positionBytes[0]);
    payload.push_back((char)positionBytes[1]);
    payload.push_back((char)positionBytes[2]);
    payload.push_back((char)positionBytes[3]);
    payload.push_back((char)speed);
    myEmulatedHub.writeValue(MessageType::PORT_VALUE_COMBINEDMODE, payload);


    // recorded sequence from real motor
    // char sensorMessage[10][8] = {
    //     {0x00, 0x00, 0x03, 0x8f, 0x03, 0x00, 0x00, 0x00},
    //     {0x00, 0x00, 0x03, 0x8c, 0x03, 0x00, 0x00, 0xf8},
    //     {0x00, 0x00, 0x03, 0x81, 0x03, 0x00, 0x00, 0xed},
    //     {0x00, 0x00, 0x03, 0x72, 0x03, 0x00, 0x00, 0xea},
    //     {0x00, 0x00, 0x03, 0x63, 0x03, 0x00, 0x00, 0xee},
    //     {0x00, 0x00, 0x03, 0x59, 0x03, 0x00, 0x00, 0xf3},
    //     {0x00, 0x00, 0x03, 0x52, 0x03, 0x00, 0x00, 0xf6},
    //     {0x00, 0x00, 0x03, 0x4b, 0x03, 0x00, 0x00, 0xf7},
    //     {0x00, 0x00, 0x03, 0x45, 0x03, 0x00, 0x00, 0xf8},
    //     {0x00, 0x00, 0x03, 0x40, 0x03, 0x00, 0x00, 0xf8},
    // };

    // for (size_t i = 0; i < 10; i++)
    // {
    //   delay(100);
    //   std::string payload;
    //   payload.append(sensorMessage[i], 8);
    //   myEmulatedHub.writeValue(MessageType::PORT_VALUE_COMBINEDMODE, payload);
    // }

    
  }

} // End of loop