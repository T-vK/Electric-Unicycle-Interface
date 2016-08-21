/* This example simply watches the speed of the unicycle
 * and sends a beep command to it when it drives faster than 4 km/h.
 */

#include <math.h>
#include <SoftwareSerial.h>
#include <EucInterface.h>

//Bluetooth serial with rx on pin 9 and tx on pin 10
SoftwareSerial BluetoothSerial(9,10);

Euc Euc(BluetoothSerial, BluetoothSerial); // Receive and transmit data via bluetooth

void setup() {
  Serial.begin(250000); // We'll use the normal hardware serial to print out all the received data
  
  BluetoothSerial.begin(9600); // Most unicycles communicate @9600 baud over bluetooth
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