/**
 * A Legoino example to control a train which has a motor connected
 * to the Port A of the Hub
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "Lpf2HubEmulation.h"



// create a hub instance
Lpf2HubEmulation myEmulatedHub;

void writeValueCallback(byte port, byte value) {
  Serial.println("writeValueCallback");
  Serial.println(port, HEX);
  Serial.println(value, HEX);
}

void setup() {
    Serial.begin(115200);
    myEmulatedHub.setWriteCallback(&writeValueCallback);
    myEmulatedHub.start();
    
} 


// main loop
void loop() {

  myEmulatedHub.initializePorts();
  
  
} // End of loop