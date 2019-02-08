# Legoino
Arduino Library for controlling Powered UP, WeDo and Boost controllers

*Disclaimer*: LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this project.

[![Legoino SimpleTrain Example](http://img.youtube.com/vi/o1hgZQz3go4/0.jpg)](http://www.youtube.com/watch?v=o1hgZQz3go4 "Legoino SimpleTrain Example")

Up to now the Library is only teseted for a Powered Up Train controller. You can connect to your HUB, set the LED color, control the motors (speed, port) and shut down the HUB via a Arduino command.

# Setup and Usage

Just install the Libray via the Arduino Library Manager and add the following include to your Arduino sketch:
```c
#include "Legoino.h"
```

Open your sketch (*.ino) and make a new instance of the Legoino object
```c
Legoino myTrainHub;
```

In the ```setup``` part of your Arduino sketch, just initialize your Legoino with the desired HUB Type
```c
myTrainHub.init(POWEREDUP);
```
Available Hub Types are: ```BOOST```, ```POWEREDUP``` and ```WEDO```

In the main ```loop``` just add the following connection flow
```c
  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("We are now connected to Train HUB");
    } else {
      Serial.println("We have failed to connect to the Train HUB");
    }
  }
```

And now you are ready to control your actuators or your LEGO Hub

## LED control

You can either define a color of the LED in the HUB via predifined colors or you can define the color via RGB values
```c
myTrainHub.setLedColor(GREEN);
```
Available colors are: BLACK, PINK, PURPLE, BLUE, LIGHT_BLUE, CYAN, GREEN, YELLOW, ORANGE, RED, WHITE

```c
myTrainHub.setLedRGBColor(255, 50, 0);
```
The ranges of the colors are from 0..255

## Motor control

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

## Set Hub Name
You can define the display name of the Hub (e.g. displayed in the PoweredUp Apps) with the following command. 
```c
char hubName[] = "myTrainHub";
myTrainHub.setHubName(hubName);
```
The maximum supported length of the character array is 14

## Shut Down Hub

If you want to shut down the LEGO Hub, you can use the following command:
```c
myTrainHub.shutDownHub();
```
The Hub will disconnect and then shut down. 

# Examples
You can find an Example called "SimpleTrain" in the "examples" folder. 

# Arduino Hardware
The library is implemented for the ESP32 Boards and does use the ESP32_BLE_Arduino Library.

# Credits
Hands up to Lego, that they have recently open-sourced the Specification
https://github.com/LEGO/lego-ble-wireless-protocol-docs

Thanks to JorgePe and all contributors for the reverse engenieering part
https://github.com/JorgePe/BOOSTreveng

Thanks to jakorten for his SWIFT iOS App
https://github.com/jakorten/UpControl

Thanks Nathan Kellenicki for his brilliant structured node module
https://github.com/nathankellenicki/node-poweredup

# Remarks
Prerequisite of that library is the BLE ESP32 Library with at least version 1.0.1 Otherwise the notifcations of changed charachteristic values will not work
https://github.com/nkolban/ESP32_BLE_Arduino

# ToDo
* Test on other controllers
* Test for all sensors/actors
* Add functionallity (fetch battery status, ... )
