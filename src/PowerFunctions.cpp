/*
* Lego Power Functions Infrared Control for Arduino
* original source from https://github.com/jurriaan/Arduino-PowerFunctions
*
* see http://www.philohome.com/pf/LEGO_Power_Functions_RC_v120.pdf for more info
* Based on SuperCow's code (http://forum.arduino.cc/index.php?topic=38142.0)
*
* Added some speed mapping functions
*
*  Released under MIT License
*
*/

// By spec CHECKSUM is a XOR based on the 4-bit triplet you are sending
#define PF_CHECKSUM() (0xf ^ _nib1 ^ _nib2 ^ _nib3)

#include <stdlib.h>
#include "PowerFunctions.h"
#include "Arduino.h"

// Aliases
void PowerFunctions::red_pwm(uint8_t pwm) { single_pwm(PF_RED, pwm); }
void PowerFunctions::blue_pwm(uint8_t pwm) { single_pwm(PF_BLUE, pwm); }

// Constructor
PowerFunctions::PowerFunctions(uint8_t pin, uint8_t channel, bool debug)
{
  _channel = channel;
  _toggle = 0;
  _pin = pin;
  _debug = debug;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

// Single output mode PWM
void PowerFunctions::single_pwm(uint8_t output, uint8_t pwm)
{
  _nib1 = _toggle | _channel;
  _nib2 = PF_SINGLE_OUTPUT | output;
  _nib3 = pwm;
  send();
  toggle();
}

void PowerFunctions::single_increment(uint8_t output)
{
  _nib1 = _toggle | _channel;
  _nib2 = PF_SINGLE_EXT | output;
  _nib3 = 0x4;
  send();
  toggle();
}

void PowerFunctions::single_decrement(uint8_t output)
{
  _nib1 = _toggle | _channel;
  _nib2 = PF_SINGLE_EXT | output;
  _nib3 = 0x5;
  send();
  toggle();
}

// Combo PWM mode
void PowerFunctions::combo_pwm(uint8_t blue_speed, uint8_t red_speed)
{
  _nib1 = PF_ESCAPE | _channel;
  _nib2 = blue_speed;
  _nib3 = red_speed;
  send();
}


//
// Private methods
//

// Pause function see "Transmitting Messages" in Power Functions PDF
void PowerFunctions::pause(uint8_t count)
{
  uint8_t pause = 0;

  if (count == 0)
  {
    pause = 4 - (_channel + 1);
  }
  else if (count < 3)
  { // 1, 2
    pause = 5;
  }
  else
  { // 3, 4, 5
    pause = 5 + (_channel + 1) * 2;
  }
  delayMicroseconds(pause * 77); //MAX_MESSAGE_LENGTH
}

// Send the start/stop bit
void PowerFunctions::start_stop_bit()
{
  send_bit();
  delayMicroseconds(PF_START_STOP); // Extra pause for start_stop_bit
}

// Send a bit
void PowerFunctions::send_bit()
{
  for (uint8_t i = 0; i < 6; i++)
  {
    digitalWrite(_pin, HIGH);
    delayMicroseconds(PF_HALF_PERIOD);
    digitalWrite(_pin, LOW);
    delayMicroseconds(PF_HALF_PERIOD);
  }
}

void PowerFunctions::send()
{
  uint8_t i, j;
  uint16_t message = _nib1 << 12 | _nib2 << 8 | _nib3 << 4 | PF_CHECKSUM();
  bool flipDebugLed = false;
  for (i = 0; i < 6; i++)
  {
    pause(i);
    start_stop_bit();
    for (j = 0; j < 16; j++)
    {
      send_bit();
      delayMicroseconds((0x8000 & (message << j)) != 0 ? PF_HIGH_PAUSE : PF_LOW_PAUSE);
    }
    start_stop_bit();
  } // for
}

inline void PowerFunctions::toggle()
{
  _toggle ^= 0x8;
}
