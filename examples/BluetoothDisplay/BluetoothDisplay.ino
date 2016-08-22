/* Receive the data the electric unicycle sends 
 * and print it to an 128x32 pixel OLED display.
 */

#include <math.h>
#include <SoftwareSerial.h>
#include <EucInterface.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//Bluetooth serial with rx on pin 9 and tx on pin 10
SoftwareSerial BluetoothSerial(9,10);

Euc Euc(BluetoothSerial, BluetoothSerial); // Receive and transmit data via bluetooth

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup() {
  Serial.begin(250000); // We'll use the normal hardware serial to print out all the received data
  
  BluetoothSerial.begin(9600); // Most unicycles communicate @9600 baud over bluetooth
  Euc.setCallback(eucLoop); // Set a callback function in which we can receive all the data the unicycle sends

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
}

void loop() {
  Euc.tick(); // This simply needs to be called regularely
}

void eucLoop(float voltage, float speed, float tempMileage, float current, float temperature, float mileage, bool dataIsNew) {
  if (dataIsNew) { // new data received
    display.clearDisplay(); 
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    
    display.setTextSize(2);
    display.print(abs((int)speed));   display.println(" km/h");
    
    display.setTextSize(1);
    display.print("Mileage: ");       display.print(mileage,3);      display.println(" km");
    display.print("Temperature: ");   display.print(temperature);    display.println(" C");
    
    display.display();
  }
}