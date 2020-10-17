/*
* Lego Power Functions Infrared Control for Arduino
* original source from https://github.com/jurriaan/Arduino-PowerFunctions
*
* see http://www.philohome.com/pf/LEGO_Power_Functions_RC_v120.pdf for more info
* Based on SuperCow's code (http://forum.arduino.cc/index.php?topic=38142.0)
**
*  Released under MIT License
*
*/

#include <stdlib.h>
#include "PowerFunctions.h"
#include "Arduino.h"

/**
 * @brief Convert speed value to the supported PWM ranges
 * @param [in] speed value -100..100 which should be converted to a PWM value
 * @return pwm value 
 */
PowerFunctionsPwm PowerFunctions::speedToPwm(byte speed)
{
  uint8_t pwm;
  if (speed == 0)
  {
    pwm = 0x08;
  }
  else if (speed <= 100)
  {
    pwm = (speed >> 4) + 1;
  }
  else
  {
    pwm = speed >> 4;
  }
  return (PowerFunctionsPwm)pwm;
}

/**
 * @brief Constructor to define the pin of the IR LED and power function channel
 * @param [in] pin Pin of the IR LED which should be used to send out power function IR signals
 * @param [in] channel IR channel 0..4 which should be used to send out signals
 */
PowerFunctions::PowerFunctions(uint8_t pin, uint8_t channel)
{
  _channel = channel;
  _toggle = 0;
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

/**
 * @brief Constructor to define the pin of the IR LED
 * @param [in] pin Pin of the IR LED which should be used to send out power function IR signals
  */
PowerFunctions::PowerFunctions(uint8_t pin)
{
  _channel = 0;
  _toggle = 0;
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

/**
 * @brief Set the pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
 * @param [in] pwm PWM signal which is applied to the output port (Use enum values PowerFunctionsPwm)
 */
void PowerFunctions::single_pwm(PowerFunctionsPort port, PowerFunctionsPwm pwm)
{
  single_pwm(port, pwm, _channel);
}

/**
 * @brief Set the pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
 * @param [in] pwm PWM signal which is applied to the output port (Use enum values PowerFunctionsPwm)
 * @param [in] channel IR channel 0..4 which should be used to send out signals
 */
void PowerFunctions::single_pwm(PowerFunctionsPort port, PowerFunctionsPwm pwm, uint8_t channel)
{
  _nib1 = _toggle | channel;
  _nib2 = PF_SINGLE_OUTPUT | (uint8_t)port;
  _nib3 = (uint8_t)pwm;
  send(channel);
  toggle();
}

/**
 * @brief increment pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
  */
void PowerFunctions::single_increment(PowerFunctionsPort port)
{
  single_increment(port, _channel);
}

/**
 * @brief increment pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
 * @param [in] channel IR channel 0..4 which should be used to send out signals
  */
void PowerFunctions::single_increment(PowerFunctionsPort port, uint8_t channel)
{
  _nib1 = _toggle | channel;
  _nib2 = PF_SINGLE_EXT | (uint8_t)port;
  _nib3 = 0x4;
  send(channel);
  toggle();
}

/**
 * @brief decrement pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
  */
void PowerFunctions::single_decrement(PowerFunctionsPort port)
{
  single_decrement(port, _channel);
}

/**
 * @brief decrement pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
 * @param [in] channel IR channel 0..4 which should be used to send out signals
  */
void PowerFunctions::single_decrement(PowerFunctionsPort port, uint8_t channel)
{
  _nib1 = _toggle | channel;
  _nib2 = PF_SINGLE_EXT | (uint8_t)port;
  _nib3 = 0x5;
  send(channel);
  toggle();
}

/**
 * @brief Set the pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
 * @param [in] pwm PWM signal which is applied to the output port (Use enum values PowerFunctionsPwm)
 */
void PowerFunctions::combo_pwm(PowerFunctionsPwm bluePwm, PowerFunctionsPwm redPwm)
{
  combo_pwm(bluePwm, redPwm, _channel);
}

/**
 * @brief Set the pwm signal on a defined port (red/blue)
 * @param [in] port The output port to which the pwm signal is transmitted (Red=0x0, Blue=0x01)
 * @param [in] pwm PWM signal which is applied to the output port (Use enum values PowerFunctionsPwm)
 * @param [in] channel IR channel 0..4 which should be used to send out signals
 */
void PowerFunctions::combo_pwm(PowerFunctionsPwm bluePwm, PowerFunctionsPwm redPwm, uint8_t channel)
{
  _nib1 = PF_ESCAPE | channel;
  _nib2 = (uint8_t)bluePwm;
  _nib3 = (uint8_t)redPwm;
  send(channel);
}

//
// Private methods
//

// Pause function see "Transmitting Messages" in Power Functions PDF
void PowerFunctions::pause(uint8_t count, uint8_t channel)
{
  uint8_t pause = 0;

  if (count == 0)
  {
    pause = 4 - (channel + 1);
  }
  else if (count < 3)
  { // 1, 2
    pause = 5;
  }
  else
  { // 3, 4, 5
    pause = 5 + (channel + 1) * 2;
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

void PowerFunctions::send(uint8_t channel)
{
  uint8_t i, j;
  uint16_t message = _nib1 << 12 | _nib2 << 8 | _nib3 << 4 | PF_CHECKSUM();
  bool flipDebugLed = false;
  for (i = 0; i < 6; i++)
  {
    pause(i, channel);
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
