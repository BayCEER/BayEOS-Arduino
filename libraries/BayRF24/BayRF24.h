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
		uint8_t res;

		res=RF24::write(getPayload(),getPacketLength());
		uint8_t curr_pa=0;
		while(! res && curr_pa<4){
			setPALevel(curr_pa);
			res=RF24::write(getPayload(),getPacketLength());
			curr_pa++;
		}

		if(_powerdown) powerDown();
		else {
			txStandBy();
			startListening();
		}

		return !res;
	}

	void init(uint64_t address,uint8_t c=0x71,uint8_t pa_level=RF24_PA_HIGH, uint8_t rate=RF24_250KBPS){
		_pipe=address;
		RF24::begin();
		setChannel(c);
		setPayloadSize(32);
		enableDynamicPayloads();
		setCRCLength( RF24_CRC_16 ) ;
		setDataRate(rate);
		setPALevel(pa_level);
		_pa_level=pa_level;
		setRetries(15,15);
		setAutoAck(true);
		if(_powerdown) powerDown() ;
	}

	bool i_available(uint8_t* pipe_nr){
		return RF24::available(pipe_nr);
	}
	uint64_t _pipe;
	uint8_t _powerdown;
	uint8_t _pa_level;
};


#endif
