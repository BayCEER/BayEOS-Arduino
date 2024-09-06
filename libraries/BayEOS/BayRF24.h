#ifndef BayRF24_h
#define BayRF24_h

#include <RF24.h>
#include <BayEOS.h>

class BayRF24 :  public BayEOS, public RF24 {
public:
	BayRF24(uint8_t _cepin, uint8_t _cspin,uint8_t powerdown=1);

	uint8_t sendPayload(void);
	void setTXAddr(uint64_t address);
	void setTXAddr(uint8_t* address);
	void init(uint64_t address,uint8_t c=0x71,rf24_pa_dbm_e pa_level=RF24_PA_HIGH, rf24_datarate_e rate=RF24_250KBPS);
	bool i_available(uint8_t* pipe_nr);
	/**
	 * Read a frame from rx (ack payload)
	 * returns 0 for success
	 * 1 == no ack payload
	 */
	uint8_t readIntoPayload(void);

	void initRadio(void);
	uint8_t _pipe[5];
	uint8_t _powerdown;
	uint8_t _pa_level;
	uint8_t _rate;
	uint8_t _channel;
};


#endif
