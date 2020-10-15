/**
 * A Legoino example to control a power functions (IR)
 * motor or LED on the blue and red port on channel 0 of the
 * IR receiver
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PowerFunctions.h"

// create a power functions instance
PowerFunctions powerFunctions(12, 0); //Pin 12, Channel 0

void setup()
{
  Serial.begin(115200);
}

// main loop
void loop()
{
  delay(1000);
  for (int i = 0; i < 110; i += 10)
  {
    PowerFunctionsPwm pwm = powerFunctions.speedToPwm(i);
    powerFunctions.single_pwm(PowerFunctionsPort::RED, pwm);
    Serial.print("single_pwm | speed: ");
    Serial.print(i);
    Serial.print(" pwm: ");
    Serial.println((uint8_t)pwm, HEX);
    delay(1000);
  }

  for (int i = 0; i < 7; i++)
  {
    Serial.println("single_decrement");
    powerFunctions.single_decrement(PowerFunctionsPort::RED);
    delay(1000);
  }

  for (int i = 0; i < 3; i++)
  {
    Serial.println("single_increment");
    powerFunctions.single_increment(PowerFunctionsPort::RED);
    delay(1000);
  }

  for (int i = 0; i < 10; i++)
  {
    Serial.println("single_decrement");
    powerFunctions.single_decrement(PowerFunctionsPort::RED);
    delay(1000);
  }


  Serial.println("combo_pwm");
  powerFunctions.combo_pwm(PowerFunctionsPwm::FORWARD2, PowerFunctionsPwm::REVERSE3);
  delay(1000);

  Serial.println("single_pwm BRAKE");
  powerFunctions.single_pwm(PowerFunctionsPort::RED, PowerFunctionsPwm::BRAKE, 0);
  powerFunctions.single_pwm(PowerFunctionsPort::BLUE, PowerFunctionsPwm::BRAKE, 0);

} // End of loop
