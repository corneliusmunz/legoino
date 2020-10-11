# Migration from 0.x.x to 1.0.0

Due to changes in the structure of the code, the existent sketches of former libraries (< version 1.0.0) has to be migrated. 

## Renaming

To get a more consistent naming, some classes and functions has been renamed

### Changes in classnames

| Old name        | New name |
| ------------- |-------------|
| BoostHub        | MoveHub |
| PoweredUpRemote        | PoweredUpRemoteHub |


### Changes in functions

to get more specific which motor command is for wich type of motor, the functions get the `<BasicMotor>` or `<TachoMotor>` prefixes.

| Old name        | New name |
| ------------- |-------------|
| `stopMotor` | `stopBasicMotor` |
| `setMotorSpeed` | `setBasicMotorSpeed` |
|`stopMotor` |`stopTachoMotor`|
|`setMotorSpeed` |`setTachoMotorSpeed`|
|`setMotorSpeedForTime` |`setTachoMotorSpeedForTime`|
|`setMotorSpeedForDegrees` |`setTachoMotorSpeedForDegrees`|
|`setMotorAbsolutePosition` |`setTachoMotorAbsolutePosition`|
|`setMotorEncoderPosition` |`setTachoMotorEncoderPosition`|


## Removal of global variables getters and button callback
In the former library version global variables was used to write sensor values. In the new version callback functions could be used to get value updates. While removing all global variables related to sensor values, the not needed getter functions are also removed. The usage of the callback functions instead is described in the next [section](#callbacks).

~~void activateHubUpdates();~~<br>
~~int getTachoMotorRotation();~~<br>
~~double getDistance();~~<br>
~~int getColor();~~<br>
~~int getRssi();~~<br>
~~int getBatteryLevel();~~<br>
~~double getHubVoltage();~~<br>
~~double getHubCurrent();~~<br>
~~int getBoostHubMotorRotation();~~<br>
~~int getTiltX();~~<br>
~~int getTiltY();~~<br>
~~int getTiltZ();~~<br>
~~int getFirmwareVersionBuild();~~<br>
~~int getFirmwareVersionBugfix();~~<br>
~~int getFirmwareVersionMajor();~~<br>
~~int getFirmwareVersionMinor();~~<br>
~~int getHardwareVersionBuild();~~<br>
~~int getHardwareVersionBugfix();~~<br>
~~int getHardwareVersionMajor();~~<br>
~~int getHardwareVersionMinor();~~<br>
~~bool isButtonPressed();~~<br>
~~bool isLeftRemoteUpButtonPressed();~~<br>
~~bool isLeftRemoteDownButtonPressed();~~<br>
~~bool isLeftRemoteStopButtonPressed();~~<br>
~~bool isLeftRemoteButtonReleased();~~<br>
~~bool isRightRemoteUpButtonPressed();~~<br>
~~bool isRightRemoteDownButtonPressed();~~<br>
~~bool isRightRemoteStopButtonPressed();~~<br>
~~bool isRightRemoteButtonReleased();~~<br>


## Callbacks
To get notified about sensor value updates (Button, Hub properties like Voltag, Rssi, Tacho motor encoder, Speedometer, Colorsensor, Distancesensor, ...), callback functions are introduced instead of polling global variables. This has many benefits. You get notified in almost realtime instead of polling. Additionally you can use e.g. several color sensors and add a specific callback for each sensor. In the former library version this was not possible.

To get a notification you have to do the following steps

### Write callback function

### Activate notifications

## Debugging

All debug logs where restructured and now the standard `log_d`, `log_w`, `log_xx` messages could be used. The log levels could be set via the arduino environment.

## New functions

* Duplo Train support
* Callbacks 
* PowerFunction (IR) functions included
* Hub Emulation
* Support of `setTachoMotorAbsolutePosition`, `setTachoMotorEncoderPosition`



