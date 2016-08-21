/* This example rotates a nice rainbow pattern around an LED strip.
 * It will rotate exactly with the speed of your unicycle.
 * It also makes all LEDs red whenever you break abruptly.
 * Just stick one of those ws2812b strips around your unicycle and you are good to go.
 */

#include <math.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include <EucInterface.h>

SoftwareSerial FakeSerial(2,3); // we're not transmitting commands to the unicycle 
                                // and we want to use the serial tx port to send data to the PC
                                // so we're providing the Euc with a fake serial for the command transmission
Euc Euc(Serial, FakeSerial); // Receive unicycle data via Serial; Don't transmit data to the unicycle.

const int BRIGHTNESS = 100; //max: 255
const int FPS_CAP = 20;
const int BREAK_SENSITIVITY = -8; // 0=max sensitivity / <0=less sensitive

const int WHEEL_DIAMETER = 14+2; // in inch

const float STRIP_1_WHEEL_COVERAGE = 0.8;  // 0.8 = strip forms 80% of a circle
const int STRIP_1_LED_COUNT = 53;
const int STRIP_1_SIGNAL_PIN = 11;


Adafruit_NeoPixel LedStrip_1 = Adafruit_NeoPixel(STRIP_1_LED_COUNT, STRIP_1_SIGNAL_PIN, NEO_GRB + NEO_KHZ800);
const int STRIP_1_VIRTUAL_LED_COUNT = STRIP_1_LED_COUNT+(float)STRIP_1_LED_COUNT*(1-STRIP_1_WHEEL_COVERAGE);

void setup() {
  Serial.begin(115200); // We'll use the normal hardware serial to print out AND received data
  FakeSerial.begin(9600);
  Euc.setCallback(eucLoop);

  LedStrip_1.setBrightness(BRIGHTNESS);
  LedStrip_1.begin();
}

void loop() {
  Euc.tick();
}

void eucLoop(float voltage, float speed, float tempMileage, float current, float temperature, float mileage, bool dataIsNew) {
  static float updateFreq = 0;
  static float acceleration = 0;
  static bool isBreaking = false;
  unsigned long now = millis();
  
  if (dataIsNew) { // new data received
    updateFreq = calcUpdateFreq(now);
    acceleration = calcAcceleration(speed,now);
    isBreaking = acceleration < BREAK_SENSITIVITY;
    //isBreaking = (current<0 && speed>0) || (current>0 && speed<0);
    
    
    Serial.print("Voltage: ");       Serial.print(voltage);      Serial.println("V");
    Serial.print("Current: ");       Serial.print(current);      Serial.println("A");
    Serial.print("Speed: ");         Serial.print(speed);        Serial.println("km/h");
    Serial.print("Acceleration: ");  Serial.print(acceleration,3); Serial.println("km/h^2");
    Serial.print("Total mileage: "); Serial.print(mileage,3);      Serial.println("km");
    Serial.print("Temp mileage: ");  Serial.print(tempMileage,3);  Serial.println("km");
    Serial.print("Temperature: ");   Serial.print(temperature);  Serial.println(" deg Celsius");
    Serial.println("------------------------");
    Serial.print(updateFreq); Serial.println(" Updates per second");
    Serial.println("");
    Serial.println("");
    
    if (isBreaking) {
      breakLight(LedStrip_1);
    }
  }
  if (!isBreaking) {
                                                      // alternatively: pattern_rainbow_gaps
    ledWheel(LedStrip_1, WHEEL_DIAMETER, STRIP_1_WHEEL_COVERAGE, speed, pattern_rainbow);
  }
}

void breakLight(Adafruit_NeoPixel &ledStripObj) {
  int ledCount = ledStripObj.numPixels();
  for (int i=0; i<=ledCount; i++) {
    ledStripObj.setPixelColor(i,0xFF0000);
  }
  ledStripObj.show();
}

uint32_t pattern_rainbow_gaps(float percentualPosition) {
  static bool state = false;
  state = !state;
  return state ? pattern_rainbow(percentualPosition) : 0;
}

uint32_t pattern_rainbow(float percentualPosition) {
  static float t = (float)1/(float)3;
  unsigned char r=0; unsigned char g=0; unsigned char b=0;
  if (percentualPosition < t) { //red-green
    r = 0xFF*(1-percentualPosition/t);
    g = 0xFF*(percentualPosition/t);
  } else if (percentualPosition < t*2) { //green-blue
    g = 0xFF*(1-((percentualPosition-t)/t));
    b = 0xFF*((percentualPosition-t)/t);
  } else { //blue-red
    b = 0xFF*(1-((percentualPosition-t*2)/t));
    r = 0xFF*((percentualPosition-t*2)/t);
  }
  uint32_t rgb = (uint32_t)r<<16 | (uint32_t)g<<8 | (uint32_t)b;
  return rgb;
}

void whiteLight(Adafruit_NeoPixel &ledStripObj) {
  int ledCount = ledStripObj.numPixels();
  for (int i=0; i<=ledCount; i++) {
    ledStripObj.setPixelColor(i,0xFFFFFF);
  }
  ledStripObj.show();
}

void ledWheel(Adafruit_NeoPixel &ledStripObj, float wheelDiameter, float wheelCoverage, float speed, uint32_t (*ledPatternFunc)(float)) { // obj, inch, led-num, percent, km/h, arr, function
  static const float KMH_TO_METERS_SEC = 3.6;
  static const float INCH_TO_METER = 0.0254;
  static const float MILLI_SECS_TO_SECS = 1000;
  
  static float lastUpdate = 0; // seconds
  static float lastOffset = 0; // led-num
  
  speed = (float)speed/(float)KMH_TO_METERS_SEC; // seconds
  wheelDiameter *= INCH_TO_METER; // meters
  float wheelCircumference = PI*wheelDiameter; // meters
  
  float now = millis()/MILLI_SECS_TO_SECS; // seconds
  float timeDelta = now-lastUpdate; // seconds

  int ledCount = ledStripObj.numPixels();
  int virtualLedCount = ledCount+(float)ledCount*(1-wheelCoverage); // led-num

  float distanceDriven = speed*timeDelta; // meters
  float rotationDone = distanceDriven/wheelCircumference; // percent
  
  float offsetInc = virtualLedCount*rotationDone; // led-num
  float theoreticalNewOffset = lastOffset+offsetInc; // led-num
  float offset = fmod(theoreticalNewOffset,virtualLedCount); // led-num
  if (offset<0)
    offset+=virtualLedCount;

  if ((int)lastOffset!=(int)offset && timeDelta>1.0/FPS_CAP) {
    for (int i=0; i<=virtualLedCount; i++) {
      int curLedNum = i+offset;
      if (curLedNum >virtualLedCount)
        curLedNum -= virtualLedCount;
    
      if (curLedNum<=ledCount) { // only set physical leds
        float percentualPosition = (float)i/(float)virtualLedCount; // percent
        uint32_t color = ledPatternFunc(percentualPosition); // RGB HEX
        ledStripObj.setPixelColor(curLedNum,color);
      }
    }
    
    ledStripObj.show();
    lastUpdate = now;
    lastOffset = offset;
  }
}

float calcAcceleration(float currentSpeed, unsigned long currentTime) {
  static float lastTime = 0;
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