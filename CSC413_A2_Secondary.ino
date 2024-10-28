/*
 * CSC 413 Assignment 2
 * Code for Secondary Arduino
 * Written by Amanda Anderson
 * October 28, 2024
 */
/********/
#include <Wire.h>

int currentLED = 0;
int eventValue = 0;

void setup() {
  // Define the LED pin as Output
  for (int i = 1; i < 14; i++) {
    pinMode(i, OUTPUT);
  }

  // Start the I2C Bus as Secondary on address 9
  Wire.begin(9); 
  
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes) {
  eventValue = Wire.read();    // read one character from the I2C
}

void loop() {
  //If value received is 1 turn on the next LED, and turn current off
  if (eventValue == 1) {
    digitalWrite(currentLED, LOW);
    currentLED = (currentLED++ % 13) + 1;
    digitalWrite(currentLED, HIGH);
    eventValue = 0;
  }
  if (eventValue == 3) {
    digitalWrite(currentLED, LOW);
    currentLED = 0;
    eventValue = 0;
  }
}
