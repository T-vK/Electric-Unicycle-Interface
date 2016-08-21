/* Receive the data the electric unicycle sends 
 * and calculate additional information like 
 * acceleration and update frequency
 * and print it to the serial console on your PC.
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
  static float updateFreq = 0;
  static float acceleration = 0;
  unsigned long now = millis();
  
  if (dataIsNew) { // new data received
    updateFreq = calcUpdateFreq(now);
    acceleration = calcAcceleration(speed,now);
    
    Serial.print("Voltage: ");       Serial.print(voltage);        Serial.println("V");
    Serial.print("Current: ");       Serial.print(current);        Serial.println("A");
    Serial.print("Speed: ");         Serial.print(speed);          Serial.println("km/h");
    Serial.print("Acceleration: ");  Serial.print(acceleration,3); Serial.println("km/h^2");
    Serial.print("Total mileage: "); Serial.print(mileage,3);      Serial.println("km");
    Serial.print("Temp mileage: ");  Serial.print(tempMileage,3);  Serial.println("km");
    Serial.print("Temperature: ");   Serial.print(temperature);    Serial.println(" deg Celsius");
    Serial.println("------------------------");
    Serial.print(updateFreq); Serial.println(" Updates per second");
    Serial.println("");
    Serial.println("");
  }
}


float calcAcceleration(float currentSpeed, unsigned long currentTime) {
  static unsigned long lastTime = 0;
  static float lastSpeed = 0;
  if (currentTime == 0)
      return 0;

  currentSpeed = abs(currentSpeed);

  float timeDelta = (float)(currentTime-lastTime)/1000; // secs
  float speedDelta = (currentSpeed-lastSpeed)/3.6; // m/s

  if (timeDelta == 0)
      return 0;
  float acceleration = speedDelta/timeDelta; // m/s^2
  lastTime = currentTime;
  lastSpeed = currentSpeed;
  return acceleration*3.6; // km/h^2
}

float calcUpdateFreq(unsigned long currentTime) { // calculate updates per second
  static unsigned long lastTime = 0;
  float ups = 1000.00/(currentTime-lastTime);
  lastTime = currentTime;
  return ups;
}