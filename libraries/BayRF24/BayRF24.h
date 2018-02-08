#ifndef BayRF24_h
#define BayRF24_h

#include <RF24.h>
#include <BayEOS.h>

class BayRF24 :  public BayEOS, public RF24 {
public:
	BayRF24(uint8_t _cepin, uint8_t _cspin,uint8_t powerdown=1);

	uint8_t sendPayload(void);
	void init(uint64_t address,uint8_t c=0x71,rf24_pa_dbm_e pa_level=RF24_PA_HIGH, rf24_datarate_e rate=RF24_250KBPS);
	bool i_available(uint8_t* pipe_nr);

	uint64_t _pipe;
	uint8_t _powerdown;
	uint8_t _pa_level;
};


#endif
