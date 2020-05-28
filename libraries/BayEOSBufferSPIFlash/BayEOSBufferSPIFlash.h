#ifndef BayEOSBufferSPIFlash_h
#define BayEOSBufferSPIFlash_h
#include <BayEOSBuffer.h>
#include <SPIFlash.h>

class BayEOSBufferSPIFlash : public BayEOSBuffer {
public:

	/**
	 * Constructor
	 */
	BayEOSBufferSPIFlash();
	/**
	 * Init SPIFlash Buffer
	 *
	 * Per default, _read_pos, _write_pos and _end are saved to the last sector of the flash storage
	 * in a way [unsigned long sig=0x0f0f0f0fL][_read_pos][_write_pos][_end_pos]
	 *
	 * flush_skip > 0 will ignore flush commands until the counter reaches flush_skip
	 *
	 */
	void init(SPIFlash& flash,uint8_t flush_skip=20);



private:
	void resetStorage(void);
	uint8_t write(const uint8_t b);
	uint8_t write(const uint8_t *b,uint8_t length);
	uint8_t write_flash(unsigned long pos, const uint8_t *b,uint8_t length);
	uint8_t seek(unsigned long pos);
	int read(void);
	int read(uint8_t *dest,int length);
	int read_flash(unsigned long pos, uint8_t *dest,int length);
	void flush(void);
	void checkEraseSector(const unsigned long start_pos, unsigned long end_pos);
	bool eraseSector(unsigned long pos);

	SPIFlash* _flash;
	unsigned long _temp;
	uint8_t _flush_skip;
	uint8_t _flush_count;
	uint8_t _flush_offset;
};
#endif
