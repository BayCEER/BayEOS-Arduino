/**
 * BayEOSBuffer based on the standard Arduino SD library
 *
 */
#ifndef BayEOSBufferSD_h
#define BayEOSBufferSD_h
#include <inttypes.h>
#include <SD.h>
#include "../BayEOS/BayEOS.h"
#include "../BayEOSBuffer/BayEOSBuffer.h"



class BayEOSBufferSD : public BayEOSBuffer {
public:
	/**
	 * Constructor ...
	 */
	BayEOSBufferSD(void):BayEOSBuffer(){};
	/**
	 * Constructor defining max file size
	 * Setting append=1 will not reset the buffer at arduino startup
	 * this is especially usefull for logger applications
	 */
	BayEOSBufferSD(unsigned long max_length,uint8_t append=0,const char *f="bayeos.db");
	/**
	 * Resets the file to zero
	 */
	void reset(void);

private:
	void resetStorage(void);
	uint8_t write(const uint8_t b);
	uint8_t write(const uint8_t *b,uint8_t length);
	uint8_t seek(unsigned long pos);
	int read(void);
	int read(uint8_t *dest,int length);
	void flush(void);
	char _filename[13];
	File _f;
};

#endif
