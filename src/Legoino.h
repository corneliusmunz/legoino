/*
 * Legoino.h - Arduino Library for controlling Powered UP and Boost controllers
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
*/

#ifndef Legoino_h
#define Legoino_h

#include "Arduino.h"
#include "BLEDevice.h"

#define LPF2_UUID "00001623-1212-efde-1623-785feabcd123"
#define LPF2_CHARACHTERISTIC "00001624-1212-efde-1623-785feabcd123"

typedef enum Port {
  A = 0x37,
  B = 0x38,
  AB = 0x39,
  C = 0x01,
  D = 0x02,
  TILT = 0x3A
};
// Boost
            // "A": new Port("A", 55),
            // "B": new Port("B", 56),
            // "AB": new Port("AB", 57),
            // "TILT": new Port("TILT", 58),
            // "C": new Port("C", 1),
            // "D": new Port("D", 2)

// Train
            // "A": new Port("A", 0),
            // "B": new Port("B", 1),
            // "AB": new Port("AB", 57)
// typedef enum Port {
//   A = 0x00,
//   B = 0x01,
//   AB = 0x39
// };


typedef void (*ButtonCallback)(bool isPressed);
typedef void (*PortCallback)(Port port, bool isConnected);

typedef enum HubType{  
    BOOST_MOVE_HUB = 2,
    POWERED_UP_HUB = 3,
    POWERED_UP_REMOTE = 4,
};

typedef enum DeviceType {
    BASIC_MOTOR = 1,
    TRAIN_MOTOR = 2,
    LED_LIGHTS = 8,
    BOOST_LED = 22,
    BOOST_DISTANCE = 37,
    BOOST_TACHO_MOTOR = 38,
    BOOST_MOVE_HUB_MOTOR = 39,
    BOOST_TILT = 40,
    POWERED_UP_REMOTE_BUTTON = 55
};

typedef enum Color {
    BLACK = 0,
    PINK = 1,
    PURPLE = 2,
    BLUE = 3,
    LIGHTBLUE = 4,
    CYAN = 5,
    GREEN = 6,
    YELLOW = 7,
    ORANGE = 8,
    RED = 9,
    WHITE = 10,
    NONE = 255
};



class Legoino
{
  public:
    Legoino();
    void init();
    bool connectHub();
    bool isConnected();
    bool isConnecting();
    
    void setHubName(char name[]);
    void shutDownHub();

    void setLedColor(Color color);
    void setLedRGBColor(char red, char green, char blue);

    void setAccelerationProfile(Port port, int16_t time, int8_t profileNumber);
    void setDecelerationProfile(Port port, int16_t time, int8_t profileNumber);

    void stopMotor(Port port);
    void setMotorSpeed(Port port, int speed);
    void setMotorSpeeds(int speedA, int speedB);
    void setMotorSpeedForTime(Port port, int speed, int16_t time);
    void setMotorSpeedForDegrees(Port port, int speed, int32_t degrees);

    void registerButtonCallback(ButtonCallback buttonCallback);
    void registerPortCallback(PortCallback portCallback);


};

#endif