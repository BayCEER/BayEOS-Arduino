//  This is a library for the K33 CO2 module connected to Arduino via I2C
#ifndef __CO2K33_H__
#define __CO2Kee_H__

#include "Arduino.h"
#include <Wire.h>


/*=========================================================================
I2C ADDRESS/BITS
-----------------------------------------------------------------------*/
#define CO2K33_ADDRESS                (0x68)
/*=========================================================================*/

class CO2K33
  {
    public:
      CO2K33(void);
      void begin(void);

/*
 * readRAM Commands
 *
 * reads specific RAM address and transforms result into float
 * returns NAN on checksum error
 */
      float readTemperature(void);
      float readCO2(void);
      float readHumidity(void);
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
      uint8_t triggerMeasurement(void);

    private:
      void wakeUp(void);
      float readRAM(uint8_t memaddr);
      uint8_t writeRAM(uint8_t memaddr,uint8_t* command, uint8_t length);
      uint8_t i;
      uint8_t buffer[4];
      int value;

  };

#endif
