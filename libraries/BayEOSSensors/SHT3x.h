/***************************************************
  This is a library for the SHT3x Digital Humidity & Temp Sensor
  with Software I2C

  it was adapted from
  https://github.com/adafruit/Adafruit_SHT31

 ****************************************************/

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <Sleep.h>


#include <SoftI2C.h>


#define SHT31_DEFAULT_ADDR    0x44
#define SHT31_MEAS_HIGHREP_STRETCH 0x2C06
#define SHT31_MEAS_MEDREP_STRETCH  0x2C0D
#define SHT31_MEAS_LOWREP_STRETCH  0x2C10
#define SHT31_MEAS_HIGHREP         0x2400
#define SHT31_MEAS_MEDREP          0x240B
#define SHT31_MEAS_LOWREP          0x2416
#define SHT31_READSTATUS           0xF32D
#define SHT31_CLEARSTATUS          0x3041
#define SHT31_SOFTRESET            0x30A2
#define SHT31_HEATEREN             0x306D
#define SHT31_HEATERDIS            0x3066


class SHT3x : private SoftI2C   {
 public:
  SHT3x(uint8_t dataPin, uint8_t clockPin, uint8_t address=SHT31_DEFAULT_ADDR );
  int8_t measure(float* t, float* h,uint8_t timeoutcounter=30,bool sleep=false);
  int8_t measureSleep(float* t, float* h,uint8_t timeoutcounter=30);
  uint16_t readStatus(void);
  void reset(void);
  void heater(boolean);
  uint8_t crc8(const uint8_t *data, int len);

 private:
  int8_t writeCommand(uint16_t cmd);
};

