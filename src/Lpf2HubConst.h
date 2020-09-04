/*
 * Lpf2HubConst.h - definition of enum types and constants
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#ifndef Lpf2HubConst_h
#define Lpf2HubConst_h

#define LPF2_UUID "00001623-1212-efde-1623-785feabcd123"
#define LPF2_CHARACHTERISTIC "00001624-1212-efde-1623-785feabcd123"

#define LPF2_VOLTAGE_MAX 9.6
#define LPF2_VOLTAGE_MAX_RAW 3893

#define LPF2_CURRENT_MAX 2444
#define LPF2_CURRENT_MAX_RAW 4095

typedef enum HubType
{
  UNKNOWNHUB = 0,
  BOOST_MOVE_HUB = 2,
  POWERED_UP_HUB = 3,
  POWERED_UP_REMOTE = 4,
  DUPLO_TRAIN_HUB = 5,
  CONTROL_PLUS_HUB = 6
};

typedef enum BLEManufacturerData
{
  DUPLO_TRAIN_HUB_ID = 32,   //0x20
  BOOST_MOVE_HUB_ID = 64,    //0x40
  POWERED_UP_HUB_ID = 65,    //0x41
  POWERED_UP_REMOTE_ID = 66, //0x42
  CONTROL_PLUS_HUB_ID = 128  //0x80
};

typedef enum DeviceType
{
  UNKNOWNDEVICE = 0,
  SIMPLE_MEDIUM_LINEAR_MOTOR = 1,
  TRAIN_MOTOR = 2,
  LIGHT = 8,
  VOLTAGE_SENSOR = 20,
  CURRENT_SENSOR = 21,
  PIEZO_BUZZER = 22,
  HUB_LED = 23,
  TILT_SENSOR = 34,
  MOTION_SENSOR = 35,
  COLOR_DISTANCE_SENSOR = 37,
  MEDIUM_LINEAR_MOTOR = 38,
  MOVE_HUB_MEDIUM_LINEAR_MOTOR = 39,
  MOVE_HUB_TILT_SENSOR = 40,
  DUPLO_TRAIN_BASE_MOTOR = 41,
  DUPLO_TRAIN_BASE_SPEAKER = 42,
  DUPLO_TRAIN_BASE_COLOR_SENSOR = 43,
  DUPLO_TRAIN_BASE_SPEEDOMETER = 44,
  TECHNIC_LARGE_LINEAR_MOTOR = 46,   // Technic Control+
  TECHNIC_XLARGE_LINEAR_MOTOR = 47,  // Technic Control+
  TECHNIC_MEDIUM_ANGULAR_MOTOR = 48, // Spike Prime
  TECHNIC_LARGE_ANGULAR_MOTOR = 49,  // Spike Prime
  TECHNIC_MEDIUM_HUB_GEST_SENSOR = 54,
  REMOTE_CONTROL_BUTTON = 55,
  REMOTE_CONTROL_RSSI = 56,
  TECHNIC_MEDIUM_HUB_ACCELEROMETER = 57,
  TECHNIC_MEDIUM_HUB_GYRO_SENSOR = 58,
  TECHNIC_MEDIUM_HUB_TILT_SENSOR = 59,
  TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR = 60,
  TECHNIC_COLOR_SENSOR = 61,              // Spike Prime
  TECHNIC_DISTANCE_SENSOR = 62,           // Spike Prime
  TECHNIC_FORCE_SENSOR = 63,              // Spike Prime
  TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY = 75, // Mindstorms
  TECHNIC_LARGE_ANGULAR_MOTOR_GREY = 76,  // Mindstorms
};

typedef enum Color
{
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



static const char *COLOR_STRING[] = {
    "black", "pink", "purple", "blue", "lightblue", "cyan", "green", "yellow", "orange", "red", "white"};

#endif