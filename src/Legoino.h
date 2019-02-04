/*
  Legoino.h - Arduino Library for controlling Powered UP and Boost controllers
  (c) Copyright 2019 - Cornelius Munz
  Released under MIT License
*/
#ifndef Legoino_h
#define Legoino_h

#include "Arduino.h"
#include "BLEDevice.h"

typedef enum HubType{  
  WEDO,
  BOOST,
  POWEREDUP
};

typedef enum Color {
    BLACK = 0,
    PINK = 1,
    PURPLE = 2,
    BLUE = 3,
    LIGHT_BLUE = 4,
    CYAN = 5,
    GREEN = 6,
    YELLOW = 7,
    ORANGE = 8,
    RED = 9,
    WHITE = 10,
    NONE = 255
};

typedef enum Port {
  A = 0x00,
  B = 0x01,
  AB = 0x39
};

#define WEDO_UUID "0001523-1212-efde-1523-785feabcd123"
#define LPF2_UUID "00001623-1212-efde-1623-785feabcd123"

#define LPF2_CHARACHTERISTIC "00001624-1212-efde-1623-785feabcd123"

class Legoino
{
  public:
    Legoino();
    void init(HubType hubType);
    bool connectHub();
    bool isConnected();
    bool isConnecting();
    
    void setLedColor(Color color);
    void setMotorSpeed(Port port, byte speed);
    void stopMotor(Port port);

};

#endif