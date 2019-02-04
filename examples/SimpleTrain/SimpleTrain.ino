/**
 * A BLE client example that is rich in capabilities.
 */

#include "Legoino.h"

Legoino myTrainHub;

void setup() {
myTrainHub.init(POWEREDUP);
} 


// This is the Arduino main loop function.
void loop() {

  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
  }

  if (myTrainHub.isConnected()) {
  
    myTrainHub.setLedColor(GREEN);

    myTrainHub.setMotorSpeed(A, 50);

    delay(1000);

    myTrainHub.stopMotor(A);

    delay(1000);

    myTrainHub.setMotorSpeed(A, -50);

    delay(1000);

    myTrainHub.stopMotor(A);

  }
  
} // End of loop
