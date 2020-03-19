#ifndef __SDP600_H__
#define __SDP600_H__

#include "Arduino.h"
#include <Wire.h>

#define SDP600_I2C_ADDR 64

class SDP600 {
public:
	void begin(int scaleFactor=1200);
	float read(void);
	void setResolution(uint8_t res=12);
	void reset(void);
private:
	void calcCRC(uint8_t value, uint8_t *crc);
	int _scaleFactor; //Depends on type - 60 for 500Pa, 240 for 125Pa, 1200 for 25Pa
};



#endif
