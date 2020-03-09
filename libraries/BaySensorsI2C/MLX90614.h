/*************************************************** 
  This is a library for the MLX90614 Temp Sensor

  Designed specifically to work with the MLX90614 sensors in the
  adafruit shop
  ----> https://www.adafruit.com/products/1748
  ----> https://www.adafruit.com/products/1749

  These sensors use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruied in any redistribution
 ****************************************************/


#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "Wire.h"


#define MLX90614_I2CADDR 0x5A

// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x0E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F

//SLEEP mode
#define MLX90614_SLEEP_MODE 0xff

class MLX90614  {
 public:
  MLX90614(uint8_t addr = MLX90614_I2CADDR);
  boolean begin();
  uint32_t readID(void);

  double readObjectTempC(uint8_t tries=3);
  double readAmbientTempC(uint8_t tries=3);
  double readObjectTempF(uint8_t tries=3);
  double readAmbientTempF(uint8_t tries=3);
  void enterSleepMode(void);


  /* *******************************************************
   * Note: Exiting sleep mode may not work for all arduinos
   * We had to use some low level Register functions
   * Testet from ATMEGA328
   *
   * *******************************************************/
  void exitSleepMode(int t_delay=100);
  uint8_t crc8 (uint8_t inCrc, uint8_t inData);

 private:
  float readTemp(uint8_t reg, uint8_t tries=3);

  uint8_t _addr;
  uint16_t read16(uint8_t addr);
  void write16(uint8_t addr, uint16_t data);
};

