#ifndef BayTCPETH_h
#define BayTCPETH_h

#define ETH_DEBUG 0

#include "../BayTCP/BayTCP.h"
#include <Ethernet.h>
#include <SPI.h>

class BayEth : protected EthernetClient, public BayTCPInterface {
public:
	/**
	 * Constructor
	 */
	BayEth(void){
		_urlencode=1;
	};
	//void begin(byte *ip,byte *mac);
	uint8_t connect(void);
	void disconnect(void);

private:
	int available(void){return EthernetClient::available();}
	int read(void){
#if ETH_DEBUG
		int c=EthernetClient::read();
		if(c!=-1) Serial.write(c);
		return c;
#else
		return EthernetClient::read();
#endif
	}
	uint8_t i_init(void){};
	int i_available(void){return EthernetClient::available();}
	size_t write(uint8_t b){
#if ETH_DEBUG
		Serial.write(b);
#endif
		return EthernetClient::write(b);
	}
    int peek(void){return EthernetClient::peek();};
    void flush(void){EthernetClient::flush();};
    void flushMTU(void){};
    void finishTransmissionMode(void){};
};

#endif
