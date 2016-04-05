#ifndef BayRF24_h
#define BayRF24_h

#include <RF24.h>

class BayRF24 :  public BayEOS, public RF24 {
public:
	BayRF24(uint8_t _cepin, uint8_t _cspin,uint8_t powerdown=1):RF24(_cepin,_cspin){
		_powerdown=powerdown;
	};

	uint8_t sendPayload(void){
		if(_powerdown) powerUp();
		else stopListening();
		openWritingPipe(_pipe);
		uint8_t res=RF24::write(getPayload(),getPacketLength());
		if(_powerdown) powerDown();
		else {
			txStandBy();
			startListening();
		}

		return !res;
	}

	void init(uint64_t address,uint8_t c=0x71,uint8_t pa_level=RF24_PA_MAX){
		_pipe=address;
		RF24::begin();
		setChannel(c);
		setPayloadSize(32);
		enableDynamicPayloads();
		setCRCLength( RF24_CRC_16 ) ;
		setDataRate(RF24_250KBPS);
		setPALevel(pa_level);
		setRetries(15,15);
		setAutoAck(true);
		if(_powerdown) powerDown() ;
	}

	bool i_available(uint8_t* pipe_nr){
		return RF24::available(pipe_nr);
	}
	uint64_t _pipe;
	uint8_t _powerdown;
};


#endif
