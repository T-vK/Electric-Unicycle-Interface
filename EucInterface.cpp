#include "EucInterface.h"

Euc::Euc(Stream &ReceiverSerial, Stream &TransmitterSerial):
ReceiverSerial(ReceiverSerial), TransmitterSerial(TransmitterSerial) {
}

void Euc::setCallback(void (*eucLoopCallback)(float,float,float,float,float,float,bool)) {
  eucLoop = eucLoopCallback;
}

void Euc::tick() {
  Euc::RawData rawData = this->receiveRawData();
  Euc::UsableData data = this->makeRawDataUsable(rawData);
  this->eucLoop(data.voltage,data.speed,data.tempMileage,data.current,data.temperature,data.mileage,data.dataIsNew);
}

Euc::UsableData Euc::makeRawDataUsable(Euc::RawData eucRawData) {
  Euc::UsableData eucUsableData;
  
  eucUsableData.voltage = (float)((eucRawData.voltage[0] << 8) | eucRawData.voltage[1])/100; // volts
  eucUsableData.speed = (float)((eucRawData.speed[0] << 8) | eucRawData.speed[1])/100*3.6; // kilometers per hour
  eucUsableData.tempMileage = (float)((uint32_t)eucRawData.tempMileage[0]<<24 | (uint32_t)eucRawData.tempMileage[1]<<16 | (uint32_t)eucRawData.tempMileage[2]<<8 | (uint32_t)eucRawData.tempMileage[3])/1000.00; // kilometers
  eucUsableData.current = (float)((eucRawData.current[0] << 8) | eucRawData.current[1])/100; // amps
  eucUsableData.temperature = (float)((eucRawData.temperature[0] << 8) | eucRawData.temperature[1])/340+36.53; // Degrees Celsius
  eucUsableData.mileage = (float)((uint32_t)eucRawData.mileage[0]<<24 | (uint32_t)eucRawData.mileage[1]<<16 | (uint32_t)eucRawData.mileage[2]<<8 | (uint32_t)eucRawData.mileage[3])/1000.00; // kilometers
  eucUsableData.dataIsNew = eucRawData.dataIsNew;
  
  return eucUsableData;
}
Euc::RawData Euc::receiveRawData() {
  static unsigned int curPos = 0; // stores the current position the packet parser
  static Euc::RawData eucRawDataReceived;
  static Euc::RawData eucRawDataReceivedValid;
  // TODO: there has to be a better way than this
  static unsigned int hppEnd = sizeof Euc::RawData().headerPrimaryPacket;
  static unsigned int hppSize = sizeof Euc::RawData().headerPrimaryPacket;
  static unsigned int voltageEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage;
  static unsigned int voltageSize = sizeof Euc::RawData().voltage;
  static unsigned int speedEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed;
  static unsigned int speedSize = sizeof Euc::RawData().speed;
  static unsigned int tempMileageEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed+sizeof Euc::RawData().tempMileage;
  static unsigned int tempMileageSize = sizeof Euc::RawData().tempMileage;
  static unsigned int currentEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed+sizeof Euc::RawData().tempMileage+sizeof Euc::RawData().current;
  static unsigned int currentSize = sizeof Euc::RawData().current;
  static unsigned int temperatureEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed+sizeof Euc::RawData().tempMileage+sizeof Euc::RawData().current+sizeof Euc::RawData().temperature;
  static unsigned int temperatureSize = sizeof Euc::RawData().temperature;
  static unsigned int unknownDataEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed+sizeof Euc::RawData().tempMileage+sizeof Euc::RawData().current+sizeof Euc::RawData().temperature+sizeof Euc::RawData().unknownData;
  static unsigned int unknownDataSize = sizeof Euc::RawData().unknownData;
  static unsigned int hspEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed+sizeof Euc::RawData().tempMileage+sizeof Euc::RawData().current+sizeof Euc::RawData().temperature+sizeof Euc::RawData().unknownData+sizeof Euc::RawData().headerSecondaryPacket;
  static unsigned int hspSize = sizeof Euc::RawData().headerSecondaryPacket;
  static unsigned int mileageEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed+sizeof Euc::RawData().tempMileage+sizeof Euc::RawData().current+sizeof Euc::RawData().temperature+sizeof Euc::RawData().unknownData+sizeof Euc::RawData().headerSecondaryPacket+sizeof Euc::RawData().mileage;
  static unsigned int mileageSize = sizeof Euc::RawData().mileage;
  static unsigned int endEnd = sizeof Euc::RawData().headerPrimaryPacket+sizeof Euc::RawData().voltage+sizeof Euc::RawData().speed+sizeof Euc::RawData().tempMileage+sizeof Euc::RawData().current+sizeof Euc::RawData().temperature+sizeof Euc::RawData().unknownData+sizeof Euc::RawData().headerSecondaryPacket+sizeof Euc::RawData().mileage+sizeof Euc::RawData().end-1;
  static unsigned int endSize = sizeof Euc::RawData().end;

  eucRawDataReceivedValid.dataIsNew = false;
  if (ReceiverSerial.available() > 0) { // if a new byte has been received
    unsigned char currentByte = ReceiverSerial.read(); // read the incoming byte
    
    if (curPos < hppEnd) { // expected byte of the primary header was received
      if (currentByte == eucRawDataReceived.headerPrimaryPacket[curPos]) {
        curPos++;
      } else {
        curPos = 0;
      }
    } else if (curPos < voltageEnd) {
      eucRawDataReceived.voltage[curPos-(voltageEnd-voltageSize)] = currentByte;
      curPos++;
    } else if (curPos < speedEnd) {
      eucRawDataReceived.speed[curPos-(speedEnd-speedSize)] = currentByte;
      curPos++;
    } else if (curPos < tempMileageEnd) {
      eucRawDataReceived.tempMileage[curPos-(tempMileageEnd-tempMileageSize)] = currentByte;
      curPos++;
    } else if (curPos < currentEnd) {
      eucRawDataReceived.current[curPos-(currentEnd-currentSize)] = currentByte;
      curPos++;
    } else if (curPos < temperatureEnd) {
      eucRawDataReceived.temperature[curPos-(temperatureEnd-temperatureSize)] = currentByte;
      curPos++;
    } else if (curPos < unknownDataEnd) {
      eucRawDataReceived.unknownData[curPos-(unknownDataEnd-unknownDataSize)] = currentByte;
      curPos++;
    } else if (curPos < hspEnd) { // expected byte of the secondary header was received
      if (currentByte == eucRawDataReceived.headerSecondaryPacket[curPos-(hspEnd-hspSize)]) {
        curPos++;
      } else {
        curPos = 0;
      }
    } else if (curPos < mileageEnd) {
      eucRawDataReceived.mileage[curPos-(mileageEnd-mileageSize)] = currentByte;
      curPos++;
    } else if (curPos < endEnd) { // expected byte of the end was received
      if (currentByte == eucRawDataReceived.end[curPos-(endEnd-endSize)]) {
        curPos++;
      } else {
        curPos = 0;
      };
    } else if (curPos == endEnd) { // all data received
      eucRawDataReceivedValid = eucRawDataReceived;
      eucRawDataReceivedValid.dataIsNew = true;
      curPos = 0;
    } else { // not sure if necessary
      curPos = 0;
    }
  }
  return eucRawDataReceivedValid;
}

void Euc::beep() {
  this->TransmitterSerial.println("b");
}
void Euc::maddenMode() {
  this->TransmitterSerial.println("h");
}
void Euc::comfortMode() {
  this->TransmitterSerial.println("f");
}
void Euc::softMode() {
  this->TransmitterSerial.println("s");
}
void Euc::calibrateAlignment() {
  // TODO: check if delays are necessary
  this->TransmitterSerial.println(",");
  this->TransmitterSerial.println("c");
  this->TransmitterSerial.println("y");
  this->TransmitterSerial.println("c");
  this->TransmitterSerial.println("y");
  this->TransmitterSerial.println("cy");
  this->TransmitterSerial.println("y");
}
void Euc::disableLevel1Alarm() {
  this->TransmitterSerial.println("u");
}
void Euc::disableLevel2Alarm() {
  this->TransmitterSerial.println("i");
}
void Euc::enableAlarms() {
  this->TransmitterSerial.println("o");
}
void Euc::enable6kmhTiltback() {
  this->TransmitterSerial.println("O");
}
void Euc::disable6kmhTiltback() {
  this->TransmitterSerial.println("I");
}