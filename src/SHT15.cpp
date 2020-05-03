
#include "SHT15.h"

SHT15::SHT15 (int dataPin, int clockPin) {
  _dataPin = dataPin;
  _clockPin = clockPin;
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin,HIGH);		//dataPin = 1
  digitalWrite(_clockPin,LOW);		//clkPin  = 0
}


#ifdef MYDEBUG
//Start Debug Functions
// ensure that serial port is setup
void SHT15::printVariables(){
  Serial.print("Variable _rawDataT: ");
  Serial.print(_rawDataT);
  Serial.print(" Variable _rawDataH: ");
  Serial.println(_rawDataH);
}
//End Debug Functions
#endif

// current temperature in degrees Fahrenheit
double SHT15::getTemperatureF() {
  _tempuratureF = ((double) _rawDataT * 0.018) - 39.4; 
  return _tempuratureF;
}

// current temperature in degrees C
double SHT15::getTemperatureC() {
  _tempuratureC = ((double) _rawDataT * 0.01) - 39.65; 
  return _tempuratureC;
}

//get linear humidity reading
double SHT15::getHumidityLinear(){
  //expecting 12 bit humidity reading
  _humdityLinear =  -2.0468 + 0.0367 * _rawDataH + -1.5955E-6 * pow(_rawDataH,2.0);
  return _humdityLinear;
}

//get true humidity reading // needs linear reading first
double SHT15::getHumidityTrue(){
  //expecting 12 bit humidity reading
  getHumidityLinear();
  _humdityTrue = (_tempuratureC-25.0) * (0.01 + 0.00008 * _rawDataH) + _humdityLinear;
  return _humdityTrue;
}

void SHT15::readSensor(){
  //read temp sensor
  _readDataRaw(0);
  delay(500);
  //read humidity sensor
  _readDataRaw(1);
}

void SHT15::serialSendDataJson(Stream& T){
  T.printlnf("{\"tempF\": %f,\"tempC\": %f,\"humidityLinear\": %f,\"humidityTrue\": %f}" ,this->getTemperatureF(),this->getTemperatureC(), this->getHumidityLinear(),this->getHumidityTrue());
  //String data = "{\"tempF\": 60.454,\"tempC\": 31.630000,\"humidityLinear\": 39.301001,\"humidityTrue\": 39.997416}";
}

// get unprocessed Sensor reading from SHT
void SHT15::_readDataRaw(bool sensor) {
  // 0 = tempurature; 1 = humidity
  int cmd = sensor ? _getHumidityCmd : _getTempCmd;

  #ifdef MYDEBUG
  Serial.printlnf("_readDataRaw cmd: %u", cmd);
  #endif

  _sendCommandSHT(cmd);
  _waitForResultSHT();
  _getData16SHT(sensor);
  _skipCrcSHT();
}

// Start SHT Transmission sequence
// Then send Command byte
// then wait for sensor acknowledge
void SHT15::_sendCommandSHT(uint8_t cmd) {

  bool ACK;
  // Transmission Start sequence
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, LOW);

  // shift out 8 bit cmd
  shiftOut(_dataPin, _clockPin, MSBFIRST, cmd);

  // Verify we get the correct ack
  digitalWrite(_clockPin, HIGH);
  pinMode(_dataPin, INPUT);
  ACK = digitalRead(_dataPin);
  if (ACK != LOW) {
    // send error message

    #ifdef MYDEBUG
    Serial.printlnf("_sendCommandSHT ACK 0 error");
    #endif

  }
  digitalWrite(_clockPin, LOW);
  ACK = digitalRead(_dataPin);
  if (ACK != HIGH) {
    // send error message here too

    #ifdef MYDEBUG
    Serial.printlnf("_sendCommandSHT ACK 1 error");
    #endif
  }
}

// poll data pin for SHT to begin data Transmission
void SHT15::_waitForResultSHT() {

  bool ACK;
  pinMode(_dataPin, INPUT);
  // poll data pin for sensor to pull low
  for (int i = 0; i < 100; i++) {
    delay(10);
    ACK = digitalRead(_dataPin);
    if (ACK == LOW) {
      break;
    }
  }

  if (ACK == HIGH) {
    // error out here
    #ifdef MYDEBUG
    Serial.printlnf("_waitForResultSHT error");
    #endif
  }
}

// recives SHT data while sending acknowledge signals
void SHT15::_getData16SHT(bool sensor) {
  // 0 = tempurature; 1 = humidity
  uint16_t *_rawData = sensor ? &_rawDataH : &_rawDataT;
  // get the MSB
  pinMode(_dataPin, INPUT);
  pinMode(_clockPin, OUTPUT);
  *_rawData = _dataShiftIn(8);

  #ifdef MYDEBUG
  Serial.printlnf("*_rawData MSB Contents: %i", *_rawData);
  #endif

  *_rawData *= 256; // shift msb over for later

  #ifdef MYDEBUG
  Serial.printlnf("*_rawData MSB Shifted Contents: %i", *_rawData);
  #endif

  // send ACK
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);

  // get the LSB and OR it
  pinMode(_dataPin, INPUT);
  *_rawData |= _dataShiftIn(8);

  #ifdef MYDEBUG
  Serial.printlnf("*_rawData Contents: %i", *_rawData);
  #endif
}

// ends SHT Transmission without sending CRC
void SHT15::_skipCrcSHT() {
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);

  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);
}

// custom shiftin function for SHT
uint32_t SHT15::_dataShiftIn(uint8_t numBits) {
  uint32_t bitval = 0;

  for (uint8_t i = 0; i < numBits; i++) {
    digitalWrite(_clockPin, HIGH);
    delay(10);
    // shift over bitval while ANDing new reading
    bitval = (bitval << 1) + digitalRead(_dataPin);
    digitalWrite(_clockPin, LOW);
  }

  return (bitval);
}
