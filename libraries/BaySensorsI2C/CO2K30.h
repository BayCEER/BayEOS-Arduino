//  This is a library for the K30 CO2 module connected to Arduino via I2C
#ifndef __CO2K30_H__
#define __CO2K30_H__

#include "Arduino.h"
#include <Wire.h>

/*=========================================================================
I2C ADDRESS/BITS
-----------------------------------------------------------------------*/
#define CO2K30_ADDRESS                (0x68)
/*=========================================================================*/

class CO2K30
  {
    public:
      CO2K30(void);
      void begin(void);

/*
 * readRAM Commands
 *
 * reads specific RAM address and transforms result into float
 * returns NAN on checksum error
 */
      float readCO2(void);
      float readABC(void);
 /*
  * writeRAM Commands
  *
  * returns
  * 0 on success
  * 1 on not complete
  * 2 on checksum error
  * 3 on other error
  */
      uint8_t zeroCalibration(void);
      uint8_t backgroundCalibration(void);
      uint8_t disableABC(void);

    private:
      float readRAM(uint8_t memaddr);
      uint8_t writeRAM(uint8_t memaddr,uint8_t* command, uint8_t length);
      float readEEPROM(uint8_t memaddr);
      uint8_t writeEEPROM(uint8_t memaddr,uint8_t* command, uint8_t length);
      uint8_t i;
      uint8_t buffer[4];
      int value;

  };

#endif
