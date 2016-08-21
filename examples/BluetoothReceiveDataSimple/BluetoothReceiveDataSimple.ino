/* Simply receive the data the electric unicycle sends 
 * and print it to the serial console on your PC.
 */

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
  if (dataIsNew) { // new data received
    Serial.print("Voltage: ");       Serial.print(voltage);        Serial.println("V");
    Serial.print("Current: ");       Serial.print(current);        Serial.println("A");
    Serial.print("Speed: ");         Serial.print(speed);          Serial.println("km/h");
    Serial.print("Total mileage: "); Serial.print(mileage,3);      Serial.println("km");
    Serial.print("Temp mileage: ");  Serial.print(tempMileage,3);  Serial.println("km");
    Serial.print("Temperature: ");   Serial.print(temperature);    Serial.println(" deg Celsius");
    Serial.println("");
    Serial.println("");
  }
}