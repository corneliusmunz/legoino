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

struct Version
{
  int Build;
  int Major;
  int Minor;
  int Bugfix;
};

enum struct HubType
{
  UNKNOWNHUB = 0,
  BOOST_MOVE_HUB = 2,
  POWERED_UP_HUB = 3,
  POWERED_UP_REMOTE = 4,
  DUPLO_TRAIN_HUB = 5,
  CONTROL_PLUS_HUB = 6,
  MARIO_HUB = 7
};

enum BLEManufacturerData
{
  DUPLO_TRAIN_HUB_ID = 32,   //0x20
  BOOST_MOVE_HUB_ID = 64,    //0x40
  POWERED_UP_HUB_ID = 65,    //0x41
  POWERED_UP_REMOTE_ID = 66, //0x42
  MARIO_HUB_ID = 67,         //0x32
  CONTROL_PLUS_HUB_ID = 128  //0x80
};

enum struct MessageHeader
{
  LENGTH = 0x00,
  HUB_ID = 0x01,
  MESSAGE_TYPE = 0x02
};

enum struct PortOutputMessage
{
  PORT_ID = 0x03,
  STARTUP_AND_COMPLETION = 0x04,
  SUB_COMMAND = 0x05
};

enum struct HubPropertyMessage
{
  PROPERTY = 0x03,
  OPERATION = 0x04,
  PAYLOAD = 0x05
};

enum struct DeviceType
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
  MARIO_HUB_GESTURE_SENSOR = 71,                 // https://github.com/bricklife/LEGO-Mario-Reveng
  MARIO_HUB_BARCODE_SENSOR = 73,          // https://github.com/bricklife/LEGO-Mario-Reveng
  MARIO_HUB_PANT_SENSOR = 74,             // https://github.com/bricklife/LEGO-Mario-Reveng
  TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY = 75, // Mindstorms
  TECHNIC_LARGE_ANGULAR_MOTOR_GREY = 76   // Mindstorms
};

enum struct MessageType
{
  HUB_PROPERTIES = 0x01,
  HUB_ACTIONS = 0x02,
  HUB_ALERTS = 0x03,
  HUB_ATTACHED_IO = 0x04,
  GENERIC_ERROR_MESSAGES = 0x05,
  HW_NETWORK_COMMANDS = 0x08,
  FW_UPDATE_GO_INTO_BOOT_MODE = 0x10,
  FW_UPDATE_LOCK_MEMORY = 0x11,
  FW_UPDATE_LOCK_STATUS_REQUEST = 0x12,
  FW_LOCK_STATUS = 0x13,
  PORT_INFORMATION_REQUEST = 0x21,
  PORT_MODE_INFORMATION_REQUEST = 0x22,
  PORT_INPUT_FORMAT_SETUP_SINGLE = 0x41,
  PORT_INPUT_FORMAT_SETUP_COMBINEDMODE = 0x42,
  PORT_INFORMATION = 0x43,
  PORT_MODE_INFORMATION = 0x44,
  PORT_VALUE_SINGLE = 0x45,
  PORT_VALUE_COMBINEDMODE = 0x46,
  PORT_INPUT_FORMAT_SINGLE = 0x47,
  PORT_INPUT_FORMAT_COMBINEDMODE = 0x48,
  VIRTUAL_PORT_SETUP = 0x61,
  PORT_OUTPUT_COMMAND = 0x81,
  PORT_OUTPUT_COMMAND_FEEDBACK = 0x82,
};

enum struct HubPropertyReference
{
  ADVERTISING_NAME = 0x01,
  BUTTON = 0x02,
  FW_VERSION = 0x03,
  HW_VERSION = 0x04,
  RSSI = 0x05,
  BATTERY_VOLTAGE = 0x06,
  BATTERY_TYPE = 0x07,
  MANUFACTURER_NAME = 0x08,
  RADIO_FIRMWARE_VERSION = 0x09,
  LEGO_WIRELESS_PROTOCOL_VERSION = 0x0A,
  SYSTEM_TYPE_ID = 0x0B,
  HW_NETWORK_ID = 0x0C,
  PRIMARY_MAC_ADDRESS = 0x0D,
  SECONDARY_MAC_ADDRESS = 0x0E,
  HARDWARE_NETWORK_FAMILY = 0x0F,
};

enum struct BatteryType
{
  NORMAL = 0x00,
  RECHARGEABLE = 0x01
};

enum struct ButtonState
{
  PRESSED = 0x01,
  RELEASED = 0x00,
  UP = 0x01,
  DOWN = 0xff,
  STOP = 0x7f
};

enum struct HubPropertyOperation
{
  SET_DOWNSTREAM = 0x01,
  ENABLE_UPDATES_DOWNSTREAM = 0x02,
  DISABLE_UPDATES_DOWNSTREAM = 0x03,
  RESET_DOWNSTREAM = 0x04,
  REQUEST_UPDATE_DOWNSTREAM = 0x05,
  UPDATE_UPSTREAM = 0x06,
};

enum ActionType
{
  SWITCH_OFF_HUB = 0x01,
  DISCONNECT = 0x02,
  VCC_PORT_CONTROL_ON = 0x03,
  VCC_PORT_CONTROL_OFF = 0x04,
  ACTIVATE_BUSY_INDICATION = 0x05,
  RESET_BUSY_INDICATION = 0x06,
  SHUTDOWN = 0x2F,
  HUB_WILL_SWITCH_OFF = 0x30,
  HUB_WILL_DISCONNECT = 0x31,
  HUB_WILL_GO_INTO_BOOT_MODE = 0x32,
};

enum Event
{
  DETACHED_IO = 0x00,
  ATTACHED_IO = 0x01,
  ATTACHED_VIRTUAL_IO = 0x02,
};

enum Color
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
  NUM_COLORS,
  NONE = 255
};

static const char *COLOR_STRING[NUM_COLORS + 1] = {
    "black", "pink", "purple", "blue", "lightblue", "cyan", "green", "yellow", "orange", "red", "white", "none"};

enum struct DuploTrainBaseSound
{
  BRAKE = 3,
  STATION_DEPARTURE = 5,
  WATER_REFILL = 7,
  HORN = 9,
  STEAM = 10
};

//https://github.com/bricklife/LEGO-Mario-Reveng/blob/master/IOType-0x4a.md
enum struct MarioPant
{
  NONE = 0x00,
  PROPELLER = 0x0A,
  TANOOKI = 0x0C,
  CAT = 0x11,
  FIRE = 0x12,
  PENGUIN = 0x14,
  NORMAL = 0x21,
  BUILDER = 0x22
};

//https://github.com/bricklife/LEGO-Mario-Reveng/blob/master/IOType-0x49.md
enum struct MarioBarcode
{
  NONE = 0xFF00,
  GOOMBA = 0x0200,
  REFRESH = 0x1400,
  QUESTION = 0x2900,
  CLOUD = 0x2E00,
  BAT = 0x7900,
  STAR = 0x7B00,
  KINGBOO = 0x8800,
  BOWSERJR = 0x9900,
  BOWSERGOAL = 0xB700,
  START = 0xB800
};

//https://github.com/bricklife/LEGO-Mario-Reveng/blob/master/IOType-0x49.md
enum struct MarioColor
{
  NONE = 0xFFFF,
  WHITE = 0x1300,
  RED = 0x1500,
  BLUE = 0x1700,
  YELLOW = 0x1800,
  BLACK = 0x1A00,
  GREEN = 0x2500,
  BROWN = 0x6A00,
  PURPLE = 0x0C01,
  UNKNOWN = 0x3801,
  CYAN = 0x4201
};

//https://github.com/sharpbrick/powered-up
enum struct MarioGesture
{
  NONE = 0x0000,
  BUMP = 0x0001,
  SHAKE = 0x0010,
  TURNING = 0x0100,
  FASTMOVE = 0x0200,
  TRANSLATION = 0x0400,
  HIGHFALLCRASH = 0x0800,
  DIRECTIONCHANGE = 0x1000,
  REVERSE = 0x2000,
  JUMP = 0x8000
};

enum struct BrakingStyle
{
  FLOAT = 0,
  HOLD = 126,
  BRAKE = 127
};

enum struct ControlPlusHubPort
{
  A = 0x00,
  B = 0x01,
  C = 0x02,
  D = 0x03,
  LED = 0x32,
  CURRENT = 0x3B,
  VOLTAGE = 0x3C,
  ACCELEROMETER = 0x61,
  GYRO = 0x62,
  TILT = 0x63
};

enum struct DuploTrainHubPort
{
  MOTOR = 0x00,
  LED = 0x11,
  SPEAKER = 0x01,
  COLOR = 0x12,
  SPEEDOMETER = 0x13,
  VOLTAGE = 0x14
};

enum struct MoveHubPort
{
  A = 0x00,
  B = 0x01,
  AB = 0x10,
  C = 0x02,
  D = 0x03,
  LED = 0x32,
  TILT = 0x3A,
  CURRENT = 0x3B,
  VOLTAGE = 0x3C
};

enum struct PoweredUpHubPort
{
  A = 0x00,
  B = 0x01,
  LED = 0x32,
  CURRENT = 0x3B,
  VOLTAGE = 0x3C
};

enum struct PoweredUpRemoteHubPort
{
  LEFT = 0x00,
  RIGHT = 0x01,
  LED = 0x34,
  VOLTAGE = 0x3B,
  RSSI = 0x3C
};

//https://github.com/bricklife/LEGO-Mario-Reveng/blob/master/IOType-0x4a.md
enum struct MarioHubPort
{
  GESTURE = 0x00,
  BARCODE = 0x01,
  PANTS = 0x02,
  VOLTAGE = 0x06
};

// https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#port-output-command-feedback-format
enum struct PortFeedbackMessage
{
  BUFFER_EMPTY_AND_COMMAND_IN_PROGRESS = 0x01,
  BUFFER_EMPTY_AND_COMMAND_COMPLETED = 0x02,
  COMMANDS_DISCARDED = 0x04,
  IDLE = 0x08,
  BUSY_FULL = 0x10,
};

#endif