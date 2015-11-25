#ifndef BayEOSBufferEEPROM_h
#define BayEOSBufferEEPROM_h
#include <inttypes.h>
#include <BayEOS.h>
#include <BayEOSBuffer.h>
#include <I2C_eeprom.h>



class BayEOSBufferEEPROM : public BayEOSBuffer {
public:

	/**
	 * Constructor
	 */
	BayEOSBufferEEPROM();
	/**
	 * Init EEPROM Buffer
	 *
	 * Per default, _read_pos, _write_pos and _end are saved to the last 12 bytes of the eeprom
	 * For long running eeproms one may reach the limit of 1.000.000 write cycles on this address!
	 * set pointer_offset=0 to disable storing of poiters
	 *
	 */
	void init(uint8_t i2c_address, unsigned long max_length,uint16_t pointer_offset=12);



private:
	void resetStorage(void){}
	uint8_t write(const uint8_t b);
	uint8_t write(const uint8_t *b,uint8_t length);
	uint8_t seek(unsigned long pos);
	int read(void);
	int read(uint8_t *dest,int length);
	void flush(void);

	I2C_eeprom _eeprom;
	unsigned long _pos;
	uint8_t _flush;
};

class BayEOSBufferMultiEEPROM : public BayEOSBuffer {
public:

	/**
	 * Constructor
	 */
	BayEOSBufferMultiEEPROM();
	/**
	 * init EEPROM Buffer
	 * set pointer_offset=0 to disable storing of poiters
	 */
	void init(uint8_t number,uint8_t* i2c_addresses, unsigned long max_length,uint16_t pointer_offset=12);



private:
	void resetStorage(void){}
	uint8_t write(const uint8_t b);
	uint8_t write(const uint8_t *b,uint8_t length);
	uint8_t seek(unsigned long pos);
	int read(void);
	int read(uint8_t *dest,int length);
	void flush(void);

	uint8_t getDevice(unsigned long pos);

	I2C_eeprom _eeprom;
	uint8_t _i2c_addresses[8];
	uint8_t _number;
	unsigned long _ee_size;
	unsigned long _ee_mask;
	unsigned long _pos;
	uint8_t _flush;
};

#endif
