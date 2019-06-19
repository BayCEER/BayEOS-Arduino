#ifndef MLX90614SOFTI2C_H
#define MLX90614SOFTI2C_H

#include "Arduino.h"
#include <SoftI2C.h>


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

class MLX90614SoftI2C : public SoftI2C   {
 public:
  MLX90614SoftI2C(uint8_t dataPin, uint8_t clockPin, uint8_t addr = MLX90614_I2CADDR);
  uint32_t readID(void);

  double readObjectTempC(void);
  double readAmbientTempC(void);
  double readObjectTempF(void);
  double readAmbientTempF(void);
  void enterSleepMode(void);


  /* *******************************************************
   * Note: Exiting sleep mode may not work for all arduinos
   * We had to use some low level Register functions
   * Testet from ATMEGA328
   *
   * *******************************************************/
  void exitSleepMode(int t_delay=100);

 private:
  float readTemp(uint8_t reg);

  uint8_t _addr;
  uint16_t read16(uint8_t addr);
  void write16(uint8_t addr, uint16_t data);
};

#endif
