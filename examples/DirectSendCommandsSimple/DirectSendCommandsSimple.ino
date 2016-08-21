/* This example simply watches the speed of the unicycle
 * and sends a beep command to it when it drives faster than 4 km/h.
 */

#include <EucInterface.h>

Euc Euc(Serial, Serial); // Receive and transmit data via serial

void setup() {
  Serial.begin(115200);
  Euc.setCallback(eucLoop); // Set a callback function in which we can receive all the data the unicycle sends
}

void loop() {
  Euc.tick(); // This simply needs to be called regularely
}

void eucLoop(float voltage, float speed, float tempMileage, float current, float temperature, float mileage, bool dataIsNew) {
  // Tell the unicycle to beep when you are faster than 4 km/h
  if (dataIsNew) {
    if (speed > 4 || speed < -4) {
      Euc.beep();
    }
  }
}