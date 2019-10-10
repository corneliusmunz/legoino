[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/legoinochat?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# Legoino
Arduino Library for controlling Powered UP and Boost controllers

*Disclaimer*: LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this project.

Simple Train example (just click the image to see the video)

[![Legoino TrainHub example](http://img.youtube.com/vi/o1hgZQz3go4/0.jpg)](http://www.youtube.com/watch?v=o1hgZQz3go4 "Legoino TrainHub example")

Simple Boost movement example (just click the image to see the video)

[![Legoino BoostHub simple movements example](http://img.youtube.com/vi/VgWObhyUmi0/0.jpg)](http://www.youtube.com/watch?v=VgWObhyUmi0 "Legoino BoostHub simple movements example")

Simple example to read in the color value and set the LED color of the hub dependently
[![Legoino BoostHub simple color sensor example](https://youtu.be/_xCd9Owy1nk/0.jpg)](https://youtu.be/_xCd9Owy1nk "Legoino BoostHub simple color sensor example")


Simple example to read in the rotation angle of the boost tacho motor and set the LED color of the hub (mapping angle to rainbow color)
[![Legoino BoostHub simple rotation sensor example](https://youtu.be/c3DHpX55uN0/0.jpg)](https://youtu.be/c3DHpX55uN0 "Legoino BoostHub simple rotation sensor example")

Simple example to read in the distance of the distance/color sensor and set the LED color of the hub dependently
[![Legoino BoostHub simple distance sensor example](https://youtu.be/TOAQtGGjZ6c/0.jpg)](https://youtu.be/TOAQtGGjZ6c "Legoino BoostHub simple distance sensor example")


Up to now the Library is only teseted for a Powered Up Train controllers and Boost controllers. You can connect to your HUB, set the LED color, set the Hub name, control the motors (speed, port, movements) and shut down the HUB via a Arduino command. Up to now the notifications of the hub and the reading of the sensors are not supported. But this feature will come in the next release.

# Examples
You can find 3 Examples called "BasicHub.ino", "BoostHub.ino" and "TrainHub.ino" in the "examples" folder. You can select the examples in your Arduino IDE via the Menu "File->Examples". 

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

### LED control

You can either define a color of the LED in the HUB via predifined colors or you can define the color via RGB values
```c
myBoostHub.setLedColor(GREEN);
```
Available colors are: BLACK, PINK, PURPLE, BLUE, LIGHT_BLUE, CYAN, GREEN, YELLOW, ORANGE, RED, WHITE

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

# Arduino Hardware
The library is implemented for the ESP32 Boards and does use the ESP32_BLE_Arduino Library.

# Credits
Hands up to Lego, that they have recently open-sourced the Specification
https://github.com/LEGO/lego-ble-wireless-protocol-docs

Thanks to [@JorgePe](https://github.com/JorgePe) and all contributors for the reverse engenieering part
https://github.com/JorgePe/BOOSTreveng

Thanks to [@jakorten](https://github.com/jakorten) for his SWIFT iOS App
https://github.com/jakorten/UpControl

Thanks [@nathankellenicki](https://github.com/nathankellenicki) for his brilliant structured node module
https://github.com/nathankellenicki/node-poweredup

# Remarks
Prerequisite of that library is the BLE ESP32 Library with at least version 1.0.1 Otherwise the notifcations of changed charachteristic values will not work
https://github.com/nkolban/ESP32_BLE_Arduino

# ToDo
* Notifications for Sensor values and Hub events
* Test for all sensors/actors
* Add functionallity (fetch battery status, ... )
