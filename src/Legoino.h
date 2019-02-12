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

#define WEDO_UUID "0001523-1212-efde-1523-785feabcd123"
#define LPF2_UUID "00001623-1212-efde-1623-785feabcd123"
#define LPF2_CHARACHTERISTIC "00001624-1212-efde-1623-785feabcd123"

typedef enum Port {
  A = 0x00,
  B = 0x01,
  AB = 0x39
};

typedef void (*ButtonCallback)(bool isPressed);
typedef void (*PortCallback)(Port port, bool isConnected);

typedef enum HubType{  
    WEDO2_SMART_HUB = 1,
    BOOST_MOVE_HUB = 2,
    POWERED_UP_HUB = 3,
    POWERED_UP_REMOTE = 4,
    DUPLO_TRAIN_HUB = 5
};

typedef enum DeviceType {
    BASIC_MOTOR = 1,
    TRAIN_MOTOR = 2,
    LED_LIGHTS = 8,
    BOOST_LED = 22,
    WEDO2_TILT = 34,
    WEDO2_DISTANCE = 35,
    BOOST_DISTANCE = 37,
    BOOST_TACHO_MOTOR = 38,
    BOOST_MOVE_HUB_MOTOR = 39,
    BOOST_TILT = 40,
    DUPLO_TRAIN_BASE_MOTOR = 41,
    DUPLO_TRAIN_BASE_SPEAKER = 42,
    DUPLO_TRAIN_BASE_COLOR = 43,
    DUPLO_TRAIN_BASE_SPEEDOMETER = 44,
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
    void init(HubType hubType);
    bool connectHub();
    bool isConnected();
    bool isConnecting();
    
    void setHubName(char name[]);
    void shutDownHub();

    void setLedColor(Color color);
    void setLedRGBColor(char red, char green, char blue);

    void stopMotor(Port port);
    void setMotorSpeed(Port port, int speed);
    void setMotorSpeeds(int speedA, int speedB);
    void setAccelerationProfile(Port port, int16_t time, int8_t profileNumber);
    void setDecelerationProfile(Port port, int16_t time, int8_t profileNumber);

    void registerButtonCallback(ButtonCallback buttonCallback);
    void registerPortCallback(PortCallback portCallback);


};

#endif