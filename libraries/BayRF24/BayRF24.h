#ifndef BayRF24_h
#define BayRF24_h

#include <RF24.h>

class BayRF24 :  public BayEOS, public RF24 {
public:
	BayRF24(uint8_t _cepin, uint8_t _cspin):RF24(_cepin,_cspin){};
	uint8_t sendPayload(void){
		powerUp() ;
		openWritingPipe(_pipe);
		uint8_t res=RF24::write(getPayload(),getPacketLength());
		//if(! res) init(_pipe);
		stopListening();
		powerDown();

		return !res;
	}

	void init(uint64_t address){
		_pipe=address;
		RF24::begin();
		setRetries(15,15);
		setChannel(0x71);
		enableDynamicPayloads();
		setCRCLength( RF24_CRC_16 ) ;
		setDataRate(RF24_250KBPS);
		setPALevel(RF24_PA_HIGH);
//		powerDown() ;
	}
	uint64_t _pipe;
};


#endif
