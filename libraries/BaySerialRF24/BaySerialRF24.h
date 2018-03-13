/*
 * This is an alternativ for sending BayEOS-Frames over RF24 using the BaySerial protocol
 *
 * BaySerial layer is on top of RF24
 * We have additional checksum and no limitation to 32 byte as for simple RF24
 * The receiver sends a acknowlege to the sender via RF24. This is done by the
 *
 *
 */


#ifndef BaySerialRF24_h
#define BaySerialRF24_h

#include <BaySerial.h>
#include <RF24.h>

class BaySerialRF24 : public BaySerialInterface {
protected:
	RF24* _radio; //Pointer to existing radio!!
	uint8_t buffer[32];
	uint8_t read_pos;
	uint8_t write_pos;
	uint8_t length;
	uint8_t pipe;
	uint8_t _flush_size;
public:
	/**
	 * Constructor
	 */
	BaySerialRF24(RF24& radio,int timeout=1000);

	int available(void);
	int i_available(void);
	void begin(long baud);
	void begin(uint8_t ch, uint8_t* rx_adr, uint8_t* tx_adr, uint8_t flush_size=24);
	void flush(void);
	void end(void);
	int read(void);
	size_t write(uint8_t c);

};


#endif
