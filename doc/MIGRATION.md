# Migration from 0.x.x to 1.0.0

Due to changes in the structure of the code, the existent sketches of former libraries (< version 1.0.0) have to be migrated. 

## Renaming

To get a more consistent naming, some classes and functions have been renamed

### Changes in classnames

| Old name        | New name |
| ------------- |-------------|
| BoostHub        | Boost |

All classes for different Hub types (like ControlPlusHub, PoweredUpHub, BoostHub, ...) were removed and all Hub Types can be controlled via the Base Lpf2Hub class. On exeption is the Boost.cpp which contains some "higher" level commands specific to boost models. But that is more a model class than a base hub or device class. 

The main difference between the hub types was formerly the assignment of port numbers. This is now covered in the following enums 
* `ControlPlusHubPort`
* `DuploTrainHubPort`
* `MoveHubPort`
* `PoweredUpHubPort`
* `PoweredUpRemoteHubPort`

The ports numbers could now be fetched with (as an example) the following line of code
```c++
ControlPlusHub::Port _port = ControlPlusHub::Port::D; // old way (version < 1.0.0)
byte portD = (byte)ControlPlusHubPort::D; // new way
```

### Changes in functions

To get more specific which motor command is for wich type of motor, the functions get the `<BasicMotor>`, `<TachoMotor>` or `<AbsoluteMotor>` prefixes.

| Old name        | New name |
| ------------- |-------------|
| `stopMotor` | `stopBasicMotor` |
| `setMotorSpeed` | `setBasicMotorSpeed` |
|`stopMotor` |`stopTachoMotor`|
|`setMotorSpeed` |`setTachoMotorSpeed`|
|`setMotorSpeedForTime` |`setTachoMotorSpeedForTime`|
|`setMotorSpeedForDegrees` |`setTachoMotorSpeedForDegrees`|
|`setMotorAbsolutePosition` |`setAbsoluteMotorPosition`|
|`setMotorEncoderPosition` |`setAbsoluteMotorEncoderPosition`|

The Tacho Motor commands now have 2 additional parameters to control the maximum Power and the Braking behaviour. Both parameters are added at the end of the functions and have default parameters so the call of the "old" parameter set should not lead to an error.

`maxPower` default value = 100, range = 0..100

`brakingStyle` default value = `BrakingStyle::BRAKE`, possible values `BrakingStyle::HOLD`, `BrakingStyle::BRAKE`, `BrakingStyle::FLOAT`


## Removal of global variables getters and button callback
In the former library version global variables were used to write sensor values. In the new version callback functions could be used to get value updates. While removing all global variables related to sensor values, the not needed getter functions are also removed. The usage of the callback functions instead is described in the next [section](#callbacks).


## Callbacks
To get notified about sensor value updates (Button, Hub properties like Voltag, Rssi, Tacho motor encoder, Speedometer, Colorsensor, Distancesensor, ...), callback functions are introduced instead of polling global variables. This has many benefits. You get notified in almost realtime instead of polling. Additionally you can use e.g. several color sensors and add a specific callback for each sensor. In the former library version this was not possible. After you read the following section you can also have a look into the examples which are included in the library. They are always a good source to find solutions/patterns for problems you want to solve.

To use the callbacks you have to do the following steps:

### Write callback function for port devices

To read in changes of devices which are attached to a Port (build in or external), you have to write a function with the following signature:
```c++
typedef void (*PortValueChangeCallback)(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData);
````

Example:
```c++
// callback function to handle updates of sensor values
void tachoMotorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  Serial.print("sensorMessage callback for port: ");
  Serial.println(portNumber, DEC);
  if (deviceType == DeviceType::MEDIUM_LINEAR_MOTOR)
  {
    int rotation = myHub->parseTachoMotor(pData);
    Serial.print("Rotation: ");
    Serial.print(rotation, DEC);
    Serial.println(" [degrees]");
    myHub->setLedHSVColor(abs(rotation), 1.0, 1.0);
  }
}
```

This function will be called if an value update appears and you can react on the new value. In this case the LED color changes depentend on the motor rotation.

### Write callback function for hub properties

To read in changes of hub properties (button, RSSI, battery level, ...), you have to write a function with the following signature:
```c++
typedef void (*HubPropertyChangeCallback)(void *hub, HubPropertyReference hubProperty, uint8_t *pData);
```

Example:
```c++
// callback function to handle updates of hub properties
void hubPropertyChangeCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  Serial.print("HubAddress: ");
  Serial.println(myHub->getHubAddress().toString().c_str());

  Serial.print("HubProperty: ");
  Serial.println((byte)hubProperty, HEX);

  if (hubProperty == HubPropertyReference::RSSI)
  {
    Serial.print("RSSI: ");
    Serial.println(myHub->parseRssi(pData), DEC);
    return;
  }

  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE)
  {
    Serial.print("BatteryLevel: ");
    Serial.println(myHub->parseBatteryLevel(pData), DEC);
    return;
  }

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    Serial.print("Button: ");
    Serial.println((byte)myHub->parseHubButton(pData), HEX);
    return;
  }
}
```

This function will be called if an value update appears and you can react on the new value. In this case it will check the hub button state (pressed, released) or the RSSI value has changed or the battery level has changed.


### Activate notifications

To get calls to your callback functions you have to register or activate it for the properties you want to get informed about updates. This has to be done after the hub is connected (not before). If you want to register updates for several properties/sensors, just add a short delay (50-100ms) after each register/activate call. 

For Port related updates you have to use the function
```c++
  void activatePortDevice(byte portNumber, byte deviceType, PortValueChangeCallback portValueChangeCallback = nullptr);
```

Example
```c++
// get notified for value updates of the device which is connected on port D
myMoveHub.activatePortDevice(portD, tachoMotorCallback);
delay(50);
myMoveHub.activateHubPropertyUpdate(HubPropertyReference::BUTTON, buttonCallback);
```

For Hub property related updates you have to use the function
```c++
  void activateHubPropertyUpdate(HubPropertyReference hubProperty, HubPropertyChangeCallback hubPropertyChangeCallback = nullptr);
```

Example
```c++
// get notified for value updates of the build in Button of the hub
myMoveHub.activateHubPropertyUpdate(HubPropertyReference::BUTTON, buttonCallback);
```


You can also check if an expected device is really connected to a port by using the new introduced function `checkPortForDevice`like in the following example

```c++
Serial.print("check ports... if needed sensor is already connected: ");
byte portForDevice = myHub.getPortForDeviceType((byte)DeviceType::COLOR_DISTANCE_SENSOR);
Serial.println(portForDevice, DEC);
// check for expected port number where the device should be connected
if (portForDevice == 1)  
{
	Serial.println("activatePortDevice");
	myHub.activatePortDevice(portB, colorDistanceSensorCallback);
}
```

## Debugging

All debug logs where restructured and now the standard `log_d`, `log_w`, `log_xx` messages could be used. The log levels could be set via the arduino environment.
