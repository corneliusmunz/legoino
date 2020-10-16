/**
 * A Legoino example to connect a Duplo Train hub and read out the color and speed 
 * sensor values 
 * 
 * Just put your Train on the Rails and push it a littlebit forward or backward. 
 * If you lift it from the track or hold it, it will stop. The color sensor readings 
 * should be directly visible in the Hub LED and dependent on the color a sound 
 * will be played.
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub myHub;

byte motorPort = (byte)DuploTrainHubPort::MOTOR;

void colorSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (deviceType == DeviceType::DUPLO_TRAIN_BASE_COLOR_SENSOR)
  {
    int color = myHub->parseColor(pData);
    Serial.print("Color: ");
    Serial.println(COLOR_STRING[color]);
    myHub->setLedColor((Color)color);

    if (color == (byte)RED)
    {
      myHub->playSound((byte)DuploTrainBaseSound::BRAKE);
    }
    else if (color == (byte)BLUE)
    {
      myHub->playSound((byte)DuploTrainBaseSound::WATER_REFILL);
    }
    else if (color == (byte)YELLOW)
    {
      myHub->playSound((byte)DuploTrainBaseSound::HORN);
    }
  }
}

void speedometerSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (deviceType == DeviceType::DUPLO_TRAIN_BASE_SPEEDOMETER)
  {
    int speed = myHub->parseSpeedometer(pData);
    Serial.print("Speed: ");
    Serial.println(speed);
    if (speed > 10)
    {
      Serial.println("Forward");
      myHub->setBasicMotorSpeed(motorPort, 50);
    }
    else if (speed < -10)
    {
      Serial.println("Back");
      myHub->setBasicMotorSpeed(motorPort, -50);
    }
    else
    {
      Serial.println("Stop");
      myHub->stopBasicMotor(motorPort);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  myHub.init();
}

// main loop
void loop()
{

  // connect flow
  if (myHub.isConnecting())
  {
    myHub.connectHub();
    if (myHub.isConnected())
    {
      Serial.println("Connected to Duplo Hub");

      delay(200);
      // connect color sensor and activate it for updates
      myHub.activatePortDevice((byte)DuploTrainHubPort::SPEEDOMETER, speedometerSensorCallback);
      delay(200);
      // connect speed sensor and activate it for updates
      myHub.activatePortDevice((byte)DuploTrainHubPort::COLOR, colorSensorCallback);
      delay(200);
      myHub.setLedColor(GREEN);
    }
    else
    {
      Serial.println("Failed to connect to Duplo Hub");
    }
  }

} // End of loop
