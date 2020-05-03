
#include "Particle.h"
#include "math.h"
// SHT15 sensor file

class SHT15 {
public:
  SHT15(int dataPin, int clockPin);
  double getTemperatureF();
  double getTemperatureC();
  double getHumidityLinear();
  double getHumidityTrue();
  void readSensor();
  void serialSendDataJson(Stream& T);
  
  #ifdef MYDEBUG
  void printVariables();
  #endif

private:
  uint8_t _clockPin;
  uint8_t _dataPin;
  uint16_t _rawDataT;
  uint16_t _rawDataH;
  double _tempuratureF;
  double _tempuratureC;
  double _humdityLinear;
  double _humdityTrue;
  uint8_t _getTempCmd = 0b00000011;
  uint8_t _getHumidityCmd = 0b00000101;
  void _readDataRaw(bool sensor);
  void _sendCommandSHT(uint8_t cmd);
  void _waitForResultSHT();
  void _getData16SHT(bool sensor);
  void _skipCrcSHT();
  uint32_t _dataShiftIn(uint8_t numBits);
};
