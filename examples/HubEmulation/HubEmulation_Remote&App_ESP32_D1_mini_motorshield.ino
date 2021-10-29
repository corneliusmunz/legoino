/** A Legoino Powered UP Hub emulation for ESP32 D1 mini with Wemos motor shield
 *  Emulated Hub connects with Powered Up App and/or Powered Up Remote
 *  based on library https://github.com/corneliusmunz/legoino
 *  and https://github.com/wemos/WEMOS_Motor_Shield_Arduino_Library
*/
#include "Lpf2Hub.h"
#include "Lpf2HubEmulation.h"
#include "LegoinoCommon.h"
#include "WEMOS_Motor.h"
#include <Wire.h>
#include <Servo.h>

// create a hub instance
Lpf2HubEmulation myEmulatedHub("ESPHub2", HubType::CONTROL_PLUS_HUB);
Lpf2Hub myRemote;
// D1 mini Motor shiled I2C default address: 0x30
// PWM frequency: 1000Hz(1kHz)
Motor M1(0x30, _MOTOR_A, 1000); //Motor A
Motor M2(0x30, _MOTOR_B, 1000); //Motor B
Servo servo;      // create servo object for steering control
int center = 95;  // center steering - depending on your setup
int pos = center;     // servo position
int Speed = 0;
int valueB = 0;
int valueA = 0;
// PU Remote Controller ports
byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
int currentSpeed = 0;
int updatedSpeed = 0;
int currentTurn = 0;
int updatedTurn = 0;
bool isInitializing = false;
bool isInitialized = false;

void writeValueCallback(byte port, byte value)
{
  Serial.print("Port: ");
  Serial.print(port); Serial.print(", value: ");
  Serial.println(value);
  if (value == 127) // fix for Powered UP app
  { value = 0;
  }
  
    valueA = value > 100 ? value-255 : value;   // converting byte (0,255) to int (-100,100)
    Speed = abs(valueA) < 10 ? 0 : abs(valueA); // for speed optional omission of values lower than 10
    Serial.print("Speed: ");
    Serial.println(valueA);
    //Run motors on ports A,B with direction based on valueA being negative or posive
    if ( valueA < 0 && port == 0){
      M1.setmotor(_CW, Speed);
    } else if ( port == 0){
      M1.setmotor(_CCW, Speed);
    }
    if ( valueA < 0 && port == 1){
      M2.setmotor(_CW, Speed);
    } else if ( port == 1) {
      M2.setmotor(_CCW, Speed);
    }

  if (port == 2) // set servo on port C
  {
    valueB = value > 100 ? value-255 : value; // converting byte (0,255) to int (-100,100)
    Serial.print("Turn: ");
    Serial.println(map(valueB, -100, 100, -90, 90));
    //set steearing servo angle
    pos = map(valueB, -100, 100, 70, 130); // map x range to servo angle range - depending on your setup (geekservo max ~270 degrees)
    servo.write(pos);
    //delay(15);
    Serial.print("Servo pos: ");
    Serial.println(pos);
  }
}

// callback function to handle updates of remote buttons
void remoteCallbackLeft(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myRemoteHub = (Lpf2Hub *)hub;
  if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON && portNumber == 0)
  {
    ButtonState buttonState = myRemoteHub->parseRemoteButton(pData);

    if (buttonState == ButtonState::UP)
    {
      updatedTurn = min(100, currentTurn + 25);
    }
    else if (buttonState == ButtonState::DOWN)
    {
      updatedTurn = max(-100, currentTurn - 25);
    }
    else if (buttonState == ButtonState::STOP)
    {
      updatedTurn = 0;
    }

    if (currentTurn != updatedTurn)
    {
      writeValueCallback(2,updatedTurn);
      currentTurn = updatedTurn;
    }
    Serial.print("Turn:");
    Serial.println(currentTurn, DEC);
  }
}

void remoteCallbackRight(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myRemoteHub = (Lpf2Hub *)hub;
  if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON && portNumber == 1)
  {
    ButtonState buttonState = myRemoteHub->parseRemoteButton(pData);

    if (buttonState == ButtonState::UP)
    {
      updatedSpeed = min(100, currentSpeed + 20);
    }
    else if (buttonState == ButtonState::DOWN)
    {
      updatedSpeed = max(-100, currentSpeed - 20);
    }
    else if (buttonState == ButtonState::STOP)
    {
      updatedSpeed = 0;
    }

    if (currentSpeed != updatedSpeed)
    {
      writeValueCallback(0,updatedSpeed);
      writeValueCallback(1,updatedSpeed);
      currentSpeed = updatedSpeed;
    }
    Serial.print("Speed:");
    Serial.println(currentSpeed, DEC);
  }
}


void setup()
{
  Serial.begin(115200);
  // define the callback function if a write message event on the characteristic occurs
  myEmulatedHub.setWritePortCallback(&writeValueCallback);
  myEmulatedHub.init();
  myEmulatedHub.start();
  Wire.begin(21,22);        // SDA, SCL pins for ESP32 D1 mini with motor shield
  servo.attach(14);         // steering servo on GPIO14 - D8 for D1 mini
  servo.write(center);      // center steering
}

// main loop
void loop()
{
  // if an app is connected, attach some devices
  if (myEmulatedHub.isConnected && !myEmulatedHub.isPortInitialized)
  {
    delay(1000);
    myEmulatedHub.isPortInitialized = true;
    delay(1000);
    myEmulatedHub.attachDevice((byte)ControlPlusHubPort::A, DeviceType::TRAIN_MOTOR);
    delay(1000);
    myEmulatedHub.attachDevice((byte)ControlPlusHubPort::B, DeviceType::TRAIN_MOTOR);
    delay(1000);
    myEmulatedHub.attachDevice((byte)ControlPlusHubPort::C, DeviceType::TRAIN_MOTOR);
    delay(1000);
    myEmulatedHub.attachDevice((byte)ControlPlusHubPort::D, DeviceType::TRAIN_MOTOR);
    delay(1000);
  }

  // connect flow for RC
  if (!myRemote.isConnecting() && !myRemote.isConnected() && !isInitialized)
  {
    Serial.println("Init Remote");
    isInitializing = true;
    myRemote.init(); // initalize the remote instance and try to connect
  }
  if (myRemote.isConnecting())
  {
    Serial.println("Connect Hub");
    myRemote.connectHub();
    if (myRemote.isConnected())
    {
      Serial.println("Connected to Remote");
      myRemote.setLedColor(GREEN);
    }
    else
    {
      Serial.println("Failed to connect to Remote");
    }
  }

  if (myRemote.isConnected() && !isInitialized)
  {
    Serial.println("System is initialized");
    isInitialized = true;
    delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
    // both activations are needed to get status updates
    myRemote.activatePortDevice(portLeft, remoteCallbackLeft);
    myRemote.activatePortDevice(portRight, remoteCallbackRight);
  }

} // End of loop
