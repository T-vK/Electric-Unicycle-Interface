# Electric Unicycle Interface Library

Arduino library to interface electric unicycles.  

## Intro

This library allows you to receive and automatically decode the data that EUCs (=Electric Unicycles) send via their Bluetooth interface.  
It has been tested on several older GotWay EUCs.  
This library can be used with serial Bluetooth modules that can act in master mode, like the HC-05.  
It can also be used to comminicate with the EUCs internal serial interface directly.   
(The internal serial interface I'm talking about are the tx/rx pins that go from the motherboard of your Unicycle to it's Bluetooth module.)  

## Features

### Receiving / calculating data

 - Read Voltage
 - Read Current
 - Read Speed
 - Read Total Mileage
 - Read Milage since last startup
 - Read Temperature

#### Using these values you can calculate: 

 - Battery status
 - Acceleration
 - If the unicycle is breaking
 - How much power (Watt) it is currently using
 - and much more...
 
### Sending commands

 - Make the unicylce beep
 - Set the unicycle to madden mode
 - Set the unicycle to comfort mode
 - Set the unicycle to soft mode
 - Set horizontal alignment
 - Turn off level 1 alarm
 - Turn off level 2 alarm
 - Turn on speed alarms
 - Enable tilt-back at 6km/h
 - Disable tilt-back at 6km/h
 
### Video: Electric Unicycle Interface Example: Led Rainbow and Breaklight
[![Video](https://img.youtube.com/vi/9l_gGwaTyRE/0.jpg)](https://www.youtube.com/watch?v=9l_gGwaTyRE)  
Code used: [examples/DirectLedWheel/DirectLedWheel.ino](examples/DirectLedWheel/DirectLedWheel.ino)

### Usage 

#### via Bluetooth

``` C++
#include <SoftwareSerial.h>
#include <EucInterface.h>

//Bluetooth serial with rx on pin 9 and tx on pin 10
SoftwareSerial BluetoothSerial(9,10);

Euc Euc(BluetoothSerial, BluetoothSerial); // Receive and transmit data via bluetooth

void setup() {
  Serial.begin(9600); // We'll use the normal hardware serial to print out all the received data
  
  BluetoothSerial.begin(9600); // Most unicycles communicate @9600 baud over bluetooth
  Euc.setCallback(eucLoop); // Set a callback function in which we can receive all the data the unicycle sends
}

void loop() {
  Euc.tick(); // This simply needs to be called regularely
}

void eucLoop(float voltage, float speed, float tempMileage, float current, float temperature, float mileage, bool dataIsNew) {
  if (dataIsNew) { // new data received
    Serial.print("Voltage: ");       Serial.print(voltage);      Serial.println("V");
    Serial.print("Current: ");       Serial.print(current);      Serial.println("A");
    Serial.print("Speed: ");         Serial.print(speed);        Serial.println("km/h");
    Serial.print("Total mileage: "); Serial.print(mileage,3);      Serial.println("km");
    Serial.print("Temp mileage: ");  Serial.print(tempMileage,3);  Serial.println("km");
    Serial.print("Temperature: ");   Serial.print(temperature);  Serial.println(" deg Celsius");
    Serial.println("");
    Serial.println("");
  }
}
```

#### directly wired to the electric unicycle
Note: SoftwareSerial only works properly with a baud of up to 9600.
If you're wired to the serial interface of the unicycle directly, you'll have to operate at a baud of 115200. 
Meaning you'll have to use your Arduino's hardware serial. 

``` C++
#include <EucInterface.h>

Euc Euc(Serial, Serial); // Receive and transmit data via the Arduino's hardware serial

void setup() {
  Serial.begin(115200);
  Euc.setCallback(eucLoop); // Set a callback function in which we can receive all the data the unicycle sends
}

void loop() {
  Euc.tick(); // This simply needs to be called regularely
}

void eucLoop(float voltage, float speed, float tempMileage, float current, float temperature, float mileage, bool dataIsNew) {
  if (dataIsNew) { // new data received
    // do something with the received data
  }
}
```

### More info

The electric unicycles I tested send their data 5 times persecond with a consistent 200 millisecond delay between them.

### Disclaimer

I will not be responsible or liable, directly or indirectly, in any way for any loss or damage of any kind incurred as a result of using this library or parts thereof.  
