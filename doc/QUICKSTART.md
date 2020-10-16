# Legoino Quickstart Tutorial

This is a quickstart step by step tutorial which will guide you to the first legoino project running on your ESP32 board. 

## Step 1: Buy an ESP32 board
To use the library you need an ESP32 microcontroller board which has build in Bluetooth. You can find dozens of suppliers of ESP32 boards. I have the following boards and have had good experiences with it.
* M5 Atom matrix or M5 Atom light (1st choice) [https://m5stack.com/collections/m5-atom/products/atom-matrix-esp32-development-kit](https://m5stack.com/collections/m5-atom/products/atom-matrix-esp32-development-kit)
* Heltec Wifi Kit 32 (2nd choice with build in OLED display) 
[https://heltec.org/project/wifi-kit-32/](https://heltec.org/project/wifi-kit-32/)


## Step 2: Install the Arduino IDE
To develop your application you have to download the Arduino Integrated Development Environment (IDE). Just open the following link and scroll down to the Development Environment section and choose the version which fits to your operating system
[https://www.arduino.cc/en/Main/Software](https://www.arduino.cc/en/Main/Software)
After the download, just install the tool

## Step 4: Install the Legoino Library
To use the Legoino library in the Arduino environment you have to install it with the Arduino Library Manager. You can find it in the Tools Menu. Open the library manager and search for `legoino` Select Install with the latest version. A popup window will appear which shows you the depentend library NimBLE-Arduino. Just select `Install all` to be prepared for you first sketch.
![Arduino Library Manager](Legoino_Library_Manager.gif)


## Step 5: Open Example Sketch
The easiest way to get a project up an running is to open an example provided by the library itself. You can open the examples of installed libraries via the `File` Menu.  In the `Examples` item you can scroll down to `Legoino` and can open the `TrainHub` example. Thats the easiest example with an [LEGO® Powered Up (88009) Hub](https://www.lego.com/product/hub-88009)
![Open Example Sketch](Open_Example_Sketch.gif)

After you have opend the sketch, you can press the arrow button on the top left to compile and upload your sketch to the ESP32 board. Before you will do this, select your Board in the Board manager and select the USB (Serial) Port where your board is connected. 

Now your sketch is uploaded to your ESP32 board and the TrainHub Example will run on your device. Now you can open the Serial Monitor to see messages via the USB(Serial) connection. If you press the hub button the device will automatically connect. The Hub LED will switch the color and a connected Motor on Port A will be turn forward and backward.

![Serial Monitor](Serial_Monitor_Example_Sketch.gif)

If you have any problems during the above steps and need support, just use the gitter chat where several Legoino users can provide help
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/legoinochat?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

If you have had no problems: :thumbsup: **CONGRATULATION!** :clap: :tada:

# Your first own sketch

After you have successfully run the first example you can go one step further and make some adaptions or your own sketch. It ist strongly dependent on what you will do but the following basic building blocks are normally needed.

## Add a Hub instance to your sketch
You have to add the Header files `Lpf2Hub.h` of the Hub to the top of the sketch.  Addtionally you need a so called instance of that Hub to access the functions of that hub. 

```c++
#include "Lpf2Hub.h"

Lpf2Hub myHub;
```

## Add a connection procedure

To setup a connection to your hub, the Hub instance has to be initialized. This will trigger a Scan procedure to look if a Lego Hub is active. If the library found an active hub, it will read out his data (Hub Address, Hub Name, etc.) and changes the state to `myHub.isConnecting() == true` 

Now you are ready to connect to the hub with the command `myHub.connectHub();`

If the library changes the state to `myHub.isConnected() == true` you are ready to go and do some cool stuff :grin:


In the ```setup``` part of your Arduino sketch, just initialize your Hub
```c++
myHub.init();
```

In the main ```loop``` just add the following connection flow
```c++
  if (myHub.isConnecting()) {
    myHub.connectHub();
    if (myHub.isConnected()) {
      Serial.println("We are now connected to the HUB");
    } else {
      Serial.println("We have failed to connect to the HUB");
    }
  }
```

## Motor/Actuator commands (Basic Motor, Tacho Motor, LED, ...)

If you want to control a motor or e.g. an LED you can find several commands to do that. It is dependent on the device you want to control which command is needed. 

| Device type        | Available commands |
| ------------- |-------------|
| Train motor #88011<br> Simple linear motor #45303<br> Duplo train motor #10874 #10875       | `stopBasicMotor`<br>`setBasicMotorSpeed` | 
| Medium linear motor #88008<br>Technic large motor #88013<br> Technic XL motor #88014<br> Build in MoveHub motor #88006      |   `setAccelerationProfile`<br>`setDecelerationProfile`<br>`stopTachoMotor`<br>`setTachoMotorSpeed`<br>`setTachoMotorSpeedForTime`<br>`setTachoMotorSpeedForDegrees`<br>`setAbsoluteMotorPosition`<br>`setAbsoluteMotorEncoderPosition`<br> | 
| Hub LEDs     |`setLedColor`<br>`setLedRGBColor`<br>`setLedHSVColor`<br>| 

## Sensor usage (Color, Distance, Tilt, ...)

To get notified about sensor value updates (Button, Hub properties like Voltage, Rssi, Tacho motor encoder, Speedometer, Colorsensor, Distancesensor, ...), callback functions are used. This callback functions are called when a value changes. After you read the following section you can also have a look into the examples which are included in the library. They are always a good source to find solutions/patterns for problems you want to solve.

To get a notification you have to do the following steps:

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
    myMoveHub.setLedHSVColor(abs(rotation), 1.0, 1.0);
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
myMoveHub.activateHubPropertyUpdat(HubPropertyReference::BUTTON, buttonCallback);
```

For Hub property related updates you have to use the function
```c++
  void activateHubPropertyUpdate(HubPropertyReference hubProperty, HubPropertyChangeCallback hubPropertyChangeCallback = nullptr);
```

Example
```c++
// get notified for value updates of the build in Button of the hub
myMoveHub.activateHubPropertyUpdat(HubPropertyReference::BUTTON, buttonCallback);
```