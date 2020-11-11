/**
 * A MoveHub basic example to connect a boost hub, set the led color of the hub 
 * dependent on the detected rotation of the boost tacho motor. Usage of callback function
 * if motor angle is changed
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"
#include <map>

// create a hub instance
Lpf2Hub myMoveHub;
byte portD = (byte)MoveHubPort::D;

std::map<byte, std::map<byte, std::string>> deviceInfo{
    {0, {
        {0, std::string{0x01, 0x01, 0x00, 0x00, 0x01, 0x00}},
        {1, std::string{0x02, 0x01, 0x00, 0x00, 0x01, 0x00}}
    }},
    {1, {
        {0, std::string{0x03, 0x01, 0x00, 0x00, 0x01, 0x00}},
        {1, std::string{0x04, 0x01, 0x00, 0x00, 0x01, 0x00}}
    }}
};
// map_type someMap{
//     {'m', {
//         {'e', 1},
//         {'f', 2}
//     }},
//     {'a', {
//         {'b', 5}
//     }}
// };

    // std::map<std::string, int> mapOfWords;
    // mapOfWords.insert(std::make_pair("earth", 1));
    // mapOfWords.insert(std::make_pair("moon", 2));

//  = {
//   { 0, { 0, "Hallo"}},
//   { 0, { 1, "Welt"}}
// };

    std::map<std::string, std::vector<int> > exampleMap =     {
                                { "Riti", { 3, 4, 5, 6 } },
                                { "Jack", { 1, 2, 3, 5 } }
                                };


void buttonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    if (myHub->parseHubButton(pData) == ButtonState::PRESSED) {
      myHub->setAbsoluteMotorEncoderPosition(portD, 0);
    }
  }
}

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

void setup()
{
  Serial.begin(115200);
  delay(200);
  myMoveHub.init(); // initalize the MoveHub instance
  
  // deviceInfo.insert( std::make_pair(0, std::make_pair(0, "Hallo") ) );
  // deviceInfo.insert( std::make_pair(0, std::make_pair(1, "Welt") ) );

// deviceInfo.insert(std::make_pair(1, "Welt"));


  Serial.print("deviceInfo[0x00][0x00]: ");
  // Serial.println(exampleMap["Riti"][0], DEC);
    Serial.println(deviceInfo[0][1].c_str());
  Serial.print("deviceInfo[0x00][0x01]: ");
  Serial.println(exampleMap["Jack"][0], DEC);

}

// main loop
void loop()
{

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myMoveHub.isConnecting())
  {
    myMoveHub.connectHub();
    if (myMoveHub.isConnected())
    {
      Serial.println("Connected to HUB");
      delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
      // connect boost tacho motor  to port d, activate sensor for updates, set callback function for rotation changes
      myMoveHub.activatePortDevice(portD, tachoMotorCallback);
      delay(50);
      myMoveHub.activateHubPropertyUpdate(HubPropertyReference::BUTTON, buttonCallback);

      myMoveHub.setLedColor(GREEN);
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myMoveHub.isConnected())
  {
    // nothing has to be done because the sensor values are received in the callback function if an update occurs
  }

} // End of loop
