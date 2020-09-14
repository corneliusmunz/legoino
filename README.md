[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/legoinochat?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/corneliusmunz/legoino)](https://github.com/corneliusmunz/legoino/releases/latest/)
# Legoino

Arduino Library for controlling Powered UP and Boost controllers

*Disclaimer*: LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this project.

*Attention*:
From version 0.7.0 on, the legoino library switched from the former BLE ESP32 Library to the new NimBLE-Arduino library (https://github.com/h2zero/NimBLE-Arduino) which is needed in version 1.0.1 So just install as a prerequesite the version 1.0.1 of that library via the Arduino Library manager or the platform.io library manager (https://www.arduinolibraries.info/libraries/nim-ble-arduino)

Remote control your boost model example (just click the image to see the video)

[![Legoino Boost control example](http://img.youtube.com/vi/UtHMKe2Insw/mqdefault.jpg)](https://youtu.be/UtHMKe2Insw "Legoino boost control example")

Simple Train example (just click the image to see the video)

[![Legoino TrainHub color control example](http://img.youtube.com/vi/GZ0fqe3-Bhw/mqdefault.jpg)](https://youtu.be/GZ0fqe3-Bhw "Legoino TrainHub color control example")

Simple Boost movement example (just click the image to see the video)

[![Legoino BoostHub simple movements example](http://img.youtube.com/vi/VgWObhyUmi0/mqdefault.jpg)](http://www.youtube.com/watch?v=VgWObhyUmi0 "Legoino BoostHub simple movements example")

Up to now the Library is only teseted for a Powered Up Train controllers, Boost controllers and Control+ Hubs. You can connect to your HUB, set the LED color, set the Hub name, control the motors (speed, port, movements) and shut down the HUB via a Arduino command. You also are able to read in hub device infos (rssi, battery level, tilt) and sensor values (color, distance, rotation angle). 

# Examples
You can find different Examples in the "examples" folder. You can select the examples in your Arduino IDE via the Menu "File->Examples". Just have a look on the videos to see the examples running :smiley: 
* **BoostHub.ino:** Example who uses the basic boost moovements (feasable for M.T.R.4 or Vernie model). http://www.youtube.com/watch?v=VgWObhyUmi0 
* **BoostHubColorSensor.ino:** Example which reads in the color Sensor value on port C and uses the detected color to set the Hub LED accordingly. https://youtu.be/_xCd9Owy1nk
* **BoostHubDeviceInfo.ino:** Example which displays the various device infos (firmware version, battery level, rssi, hardwar version, tilt) in the serial monitor
* **BoostHubDistanceSensor.ino:** Example which reads in the input of the distance sencor and set the Hub LED color dependent on the distance. https://youtu.be/TOAQtGGjZ6c 
* **BoostHubRotationSensor.ino:** Example which reads in the input of the Tacho motor angle to set the Hub LED dependent on the angle to the scale of rainbow colors. https://youtu.be/c3DHpX55uN0
* **TrainHub.ino:** Example for a PowererdUp Hub to set the speed of a train model. http://www.youtube.com/watch?v=o1hgZQz3go4 
* **TrainColor.ino:** Example of PoweredUp Hub combined with color sensor to control the speed of the train dependent on the detected color. https://youtu.be/GZ0fqe3-Bhw
* **PoweredUpRemoteAutoDetection.ino:** Example of connection of PoweredUp and PoweredUpRemote where the device type is fetched automatically and the order in which you switched on the hubs is no longer relevant. 
* **ControlPlusHub.ino:** Example of connection of ControlPlusHub (TechnicHub) where a Tacho Motor on Port D is controlled.

# Setup and Usage
Just install the Library via the Arduino Library Manager.

The usage is dependent on your hub type. Some basic commands are shared for the hubs and are covered in the Lpf2Hub library. Some other commands ar hub specific (e.g. Boost movement).

## Boost Hub
Add the follwoing include in your *.ino sketch
```c
#include "BoostHub.h"
```
Make a new instance of the Hub object
```c
BoostHub myBoostHub;
```

In the ```setup``` part of your Arduino sketch, just initialize your Hub
```c
myBoostHub.init();
```
Alternatively you can use the ```init()``` function also in the main loop if you want to check the ```isConnected()``` status e.g. to reconnect automatically the hub.

If you want to connect to a specific hub you can initialize your Hub with a specific address. The address has to be 
represented by a hex string of the format: ```00:00:00:00:00:00```
```c
myBoostHub.init("90:84:2b:03:19:7f");
```

If you want to change the BLE scan duration to detect a new Hub you can use the init function with a time parameter in seconds. The default value is 1 second. In former versions of the library it was 30 seconds which sometimes led to errors during the connection process.
```c
myBoostHub.init(2); // 2 seconds scan duration
```

Alternatively you can set the scan duration and the specific address of a hub in the init function with two parameters.
```c
myBoostHub.init("90:84:2b:03:19:7f", 2); // connect to hub with address and a scan duration of 2 seconds
```

In the main ```loop``` just add the following connection flow
```c
  if (myBoostHub.isConnecting()) {
    myBoostHub.connectHub();
    if (myBoostHub.isConnected()) {
      Serial.println("We are now connected to the HUB");
    } else {
      Serial.println("We have failed to connect to the HUB");
    }
  }
```

Now you are ready to control your actuators on your Hub

### Hub control
You can define the display name of the Hub (e.g. displayed in the PoweredUp Apps) with the following command. 
```c
char hubName[] = "myBoostHub";
myBoostHub.setHubName(hubName);
```
The maximum supported length of the character array is 14

If you want to shut down the LEGO Hub, you can use the following command:
```c
myBoostHub.shutDownHub();
```
The Hub will disconnect and then shut down. 

After the Hub is connected you can get the address of the Hub as a ```NimBLEAddress``` structure using the following command
```
myBoostHub.getHubAddress();
```
To print it out you can e.g. use the following commands
```
Serial.print("Hub address: ");
Serial.println(myBoostHub.getHubAddress().toString().c_str());
```


### LED control

You can either define a color of the LED in the HUB via predifined colors or you can define the color via RGB values
```c
myBoostHub.setLedColor(GREEN);
```
Available colors are: BLACK, PINK, PURPLE, BLUE, LIGHTBLUE, CYAN, GREEN, YELLOW, ORANGE, RED, WHITE

```c
myBoostHub.setLedRGBColor(255, 50, 0);
```
The ranges of the colors are from 0..255

### Motor control

You can define the port and speed of a motor which is connected to your HUB. The speed values vary from -100...100. 0 will stop the Motor. If you use negative values the direction is reversed. 
```c
myBoostHub.setMotorSpeed(A, 25); // 25% forward speed, Port A
myBoostHub.setMotorSpeed(A, -30); // 30% reversed speed, Port A
```

If you want to stop the motor, you can use the follwing command. If you do not specify a port value, all motors will be stopped.
```c
myBoostHub.stopMotor(A); // Stop motor on Port A
myBoostHub.stopMotor(); // Stop all motors (Port A and Port B)
```

If you want to set the motor speed for a port for a specific time in miliseconds, you can use the following command.
```c
myBoostHub.setMotorSpeedForTime(A, 50, 1000); // 50% speed, Port A, 1000ms duration
myBoostHub.setMotorSpeedForTime(A, -25, 500); // -50% speed (reversed), Port A, 500ms duration
```

If you want to set the motor speed for a port for a specific angle in degrees, you can use the following command.
```c
myBoostHub.setMotorSpeedForDegrees(A, 50, 90); // 50% speed, Port A, 90 degrees rotation 
myBoostHub.setMotorSpeedForDegrees(A, -25, 360); // -50% speed (reversed), Port A, 360 degrees rotation
```

If you want to set the motor speeds for the hub motors A,B for a specific angle in degrees, you can use the following command. Be aware, that here the so called [tacho math](https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#tacho-math) from the lego wireless protocoll specification will be applied
```c
myBoostHub.setMotorSpeedsForDegrees(50, -50, 360); // speed motor A 50%, speed motor B -50%, for 360 degrees. This will lead to a rotation 
myBoostHub.setMotorSpeedsForDegrees(50, 25, 180); // speed motor A 50%, speed motor B 25%, for 180 degrees. This will lead to a arc movement 
```

### Sensor values

If you want to read in sensor values, you first have to activate the updates for sensor values with the following command
```c
// activate color/distance sensor on port c for updates
myBoostHub.activatePortDevice(0x02, 37);
// activate tacho motor on port d for updates
myBoostHub.activatePortDevice(0x03, 38);
```
Every sensor has its own device type value (second parameter). You can find the mapping in the ```Lpf2Hub.h``` file. You should activate the sensor updates after a successful connection to the hub. 

If you have activated the sensor value updates, you can fetch the current values with the following available commands
```c
int getTachoMotorRotation(); // continious angle in degrees
double getDistance(); // distance approximation in mm
int getColor();
int getRssi(); // dB
int getBatteryLevel(); // %
int getBoostHubMotorRotation(); //continious angle in degrees
int getTiltX(); // angle
int getTiltY(); // angle
int getFirmwareVersionBuild();
int getFirmwareVersionBugfix();
int getFirmwareVersionMajor();
int getFirmwareVersionMinor();
int getHardwareVersionBuild();
int getHardwareVersionBugfix();
int getHardwareVersionMajor();
int getHardwareVersionMinor();
bool isButtonPressed();
```

Up to a limitation of the BLE library implementation it is not possible to use callback functions which are member functions of a class. Therefore, the implementation is based on public variables and you have to poll the values in your loop and will not be notified when an upadte is available. 

### Basic movements (Vernie, M.T.R. 4)
If you want to move Vernie or M.T.R. 4 you can use the following commands. These commands are using the underlying basic motor commands and are adjusted to the boost grid map.

If you want to move forward for a specific number of steps, just use the follwing command
```c
myBoostHub.moveForward(1) // move one step forward
myBoostHub.moveForward(3) // move three steps forward
```

If you want to move back for a specific number of steps, just use the follwing command
```c
myBoostHub.moveBack(1) // move one step back
myBoostHub.moveBack(3) // move three steps back
```

If you want to rotate for a specific angle, just use the follwing commands
```c
myBoostHub.rotateLeft(90) // rotate 90 degrees left
myBoostHub.rotateRight(180) // rotate 180 degrees right
myBoostHub.rotate(360) // rotate 360 degrees right (positive angles means right, negative means left)
myBoostHub.rotate(-180) // rotate 180 degrees left (positive angles means right, negative means left)
```

If you want to move with an arc for a specific angle, just use the follwing commands
```c
myBoostHub.moveArcLeft(180) // move with an arc for 180 degrees to the left
myBoostHub.moveArcRight(90) // move with an arc for 90 degrees to the right
myBoostHub.moveArc(270) // move with an arc for 270 degrees to the right (positive angles means right, negative means left)
myBoostHub.moveArc(-90) // move with an arc for 90 degrees to the left (positive angles means right, negative means left)
```

## PoweredUp Hub (Trains, Batmobile)
Add the follwoing include in your *.ino sketch

```c
#include "PoweredUpHub.h"
```

Make a new instance of the Hub object
```c
PoweredUpHub myTrainHub;
```

In the ```setup``` part of your Arduino sketch, just initialize your Hub
```c
myTrainHub.init();
```

In the main ```loop``` just add the following connection flow
```c
  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("We are now connected to the HUB");
    } else {
      Serial.println("We have failed to connect to the HUB");
    }
  }
```
Now you are ready to control your actuators on your Hub

### Hub control
You can define the display name of the Hub (e.g. displayed in the PoweredUp Apps) with the following command. 
```c
char hubName[] = "myTrainHub";
myTrainHub.setHubName(hubName);
```
The maximum supported length of the character array is 14

If you want to shut down the LEGO Hub, you can use the following command:
```c
myTrainHub.shutDownHub();
```
The Hub will disconnect and then shut down. 

### LED control

You can either define a color of the LED in the HUB via predifined colors or you can define the color via RGB values
```c
myTrainHub.setLedColor(GREEN);
```
Available colors are: BLACK, PINK, PURPLE, BLUE, LIGHT_BLUE, CYAN, GREEN, YELLOW, ORANGE, RED, WHITE

```c
myTrainHub.setLedRGBColor(255, 50, 0);
```
The ranges of the colors are from 0..255

### Motor control

You can define the port and speed of a motor which is connected to your HUB. The speed values vary from -100...100. 0 will stop the Motor. If you use negative values the direction is reversed. 
```c
myTrainHub.setMotorSpeed(A, 25); // 25% forward speed, Port A
myTrainHub.setMotorSpeed(A, -30); // 30% reversed speed, Port A
```

If you want to stop the motor, you can use the follwing command. If you do not specify a port value, all motors will be stopped.
```c
myTrainHub.stopMotor(A); // Stop motor on Port A
myTrainHub.stopMotor(); // Stop all motors (Port A and Port B)
```

## Control+ Hubs (Lego technic hubs)
Add the follwoing include in your *.ino sketch

```c
#include "ControlPlusHub.h"
```

All the other stuff is the same as for the other Hub types.

# Arduino Hardware
The library is implemented for the ESP32 Boards and does use the ESP32 NimBLE-Arduino Library.

# Connection to more than 3 hubs
It is possible to connect to up to 9 hubs in parallel with an common ESP32 board. To enable the connection to more than 3 hubs, you have to change a single configuration of the NimBLE library. Just open the ```nimconfig.h``` file located in your arduino library folder in the directory ```NimBLE-Arduino/src```. Open the file with an editor and change the following settings to your demands
```
/** @brief Sets the number of simultaneous connections (esp controller max is 9) */
#define CONFIG_BT_NIMBLE_MAX_CONNECTIONS 3
```

Then close the Arduino environment and open it again to force the rebuild of the library. Open your sketch build and upload it and be happy with multiple connections.

# Credits
Hands up to Lego, that they have recently open-sourced the Specification
https://github.com/LEGO/lego-ble-wireless-protocol-docs

Thanks to [@JorgePe](https://github.com/JorgePe) and all contributors for the reverse engenieering part
https://github.com/JorgePe/BOOSTreveng

Thanks to [@jakorten](https://github.com/jakorten) for his SWIFT iOS App
https://github.com/jakorten/UpControl

Thanks [@nathankellenicki](https://github.com/nathankellenicki) for his brilliant structured node module
https://github.com/nathankellenicki/node-poweredup

Thanks to [@h2zero](https://github.com/h2zero/NimBLE-Arduino) for developing a new BLE library based on the NimBLE project and supporting legoino with the posibility in changing the callback functions that it works also for member functions.

# Remarks
Prerequisite of that library is the NimBLE-Arduino library (https://github.com/h2zero/NimBLE-Arduino) with at least version 1.0.1 Otherwise the notifcations of changed charachteristic values will not work.

# ToDo
* Virtual Ports
* Notification for sensor value updates
