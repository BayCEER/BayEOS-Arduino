#include "BayTCPEth.h"


/*
void BayEth::begin(byte *ip,byte *mac){
	Ethernet.begin(mac, ip);
}
*/

uint8_t BayEth::connect(void){
/*	uint8_t server[4];
	uint8_t offset=0;
#if BayTCP_DEBUG_INPUT
	Serial.println(_server);
	Serial.println(atoi(_port));
		Serial.println(_config_buffer);
#endif
	for(uint8_t i=0;i<4;i++){
		server[i]=atoi(_server+offset);
		while(_server[offset] && _server[offset]!='.') offset++;
		offset++;
#if BayTCP_DEBUG_INPUT
		Serial.print(server[i]);
		Serial.print("-");

#endif
	}
	*/
	if(! EthernetClient::connect(parseIP(_server), atoi(_port))){
#if BayTCP_DEBUG_INPUT
		for(uint8_t i=0;i<4;i++){
			Serial.print(*(parseIP(_server)+i));
			Serial.print(":");
		}
		Serial.println();
#endif

		return(1);
	}
	return(0);
}

void BayEth::disconnect(void){
	EthernetClient::stop();
}

