/*
 * Implemetation of BayEOSBuffer using RAM
 */
#ifndef BayEOSBufferRAM_h
#define BayEOSBufferRAM_h
#include <inttypes.h>
#include <BayEOS.h>
#include <BayEOSBuffer.h>




class BayEOSBufferRAM : public BayEOSBuffer {
public:
	/**
	 * Constructor ...
	 */
	BayEOSBufferRAM(void){};

	/**
	 * Constructor using dynamic memory allocation
	 * Note: setting min_free to 0 may result in problems with strings
	 */
	BayEOSBufferRAM(uint16_t max_length,uint16_t min_free=100);

	/**
	 * Constructor using a allocated buffer
	 */
	BayEOSBufferRAM(uint8_t* buffer,unsigned long max_length);


private:
	void resetStorage(void){}
	uint8_t write(const uint8_t b);
	uint8_t write(const uint8_t *b,uint8_t length);
	uint8_t seek(unsigned long pos);
	int read(void);
	int read(uint8_t *dest,int length);
	void flush(void);

	uint8_t* _buffer;
	uint8_t* _ram_pos;
};

#endif
