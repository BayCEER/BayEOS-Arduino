#ifndef MAX11212_H
#define MAX11212_H

#include "Arduino.h"

class MAX11212{
private:
	uint8_t _sck;	// Power Down and Serial Clock Input Pin
	uint8_t _dout;		// Serial Data Output Pin
public:
	MAX11212(uint8_t sck,uint8_t dout);
	long read(bool cal=0,int timeout=100);
	void begin(void);
};



#endif
