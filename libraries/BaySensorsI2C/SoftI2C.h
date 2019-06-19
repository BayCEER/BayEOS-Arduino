#ifndef SOFTI2C_H
#define SOFTI2C_H

#include "Arduino.h"

#define PULSE_LONG  delayMicroseconds(30)
#define PULSE_SHORT delayMicroseconds(15)
// Function return code definitions
const int8_t S_Err_TO     = 3; // Timeout
const int8_t S_Err_CRC    = 2; // CRC failure
const int8_t S_Err_NoACK  = 1; // ACK expected but not received

class SoftI2C {
 public:
  SoftI2C(uint8_t dataPin, uint8_t clockPin);
 protected:
  void begin(void);
  void startTransmission(void);
  void stopTransmission(void);
  uint8_t beginTransmission(uint8_t addr);
  uint8_t requestFrom(uint8_t addr);
  uint8_t read(bool ack=false);
  int8_t write(uint8_t value);

  uint8_t   _pinData;         // Pin interface
  uint8_t   _pinClock;
  uint8_t _i2caddr;
};



#endif
