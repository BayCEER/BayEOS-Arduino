/**
 * BayEOSBuffer based on the SdFat library
 */
#ifndef BayEOSBufferSDFat_h
#define BayEOSBufferSDFat_h
#include <inttypes.h>
#include "../SdFat/SdFat.h"
#include "../BayEOS/BayEOS.h"
#include "../BayEOSBuffer/BayEOSBuffer.h"


class BayEOSBufferSDFat : public BayEOSBuffer {
public:
	/**
	 * Constructor ...
	 */
	BayEOSBufferSDFat(void):BayEOSBuffer(){};
	/**
	 * Constructor defining max file size
	 * Setting append=1 will not reset the buffer at arduino startup
	 * this is especially usefull for logger applications
	 */
	BayEOSBufferSDFat(unsigned long max_length,uint8_t append=0,const char *p="/bayeos.db");

private:
	void resetStorage(void);
	uint8_t write(const uint8_t b);
	uint8_t write(const uint8_t *b,uint8_t length);
	uint8_t seek(unsigned long pos);
	int read(void);
	int read(uint8_t *dest,int length);
	void flush(void);
	char _path[14];
	SdFile _f;
};

extern SdFat SD;// create usable instance

#endif
