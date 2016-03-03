#include "BayXBee.h"

BayXBeeInterface::BayXBeeInterface(uint8_t sleep_pin, uint8_t wakeup_time, int wait_time_for_response, uint16_t destination) : XBeeInterface(){
	_tx=Tx16Request(BAYXBEE_GATEWAY, _payload, 100);
	_txStatus = TxStatusResponse();
	_sleepPin=sleep_pin;
	_wakeupTime=wakeup_time;
	_destination=destination;
	_waitTimeForResponse=wait_time_for_response;
}


void BayXBeeInterface::begin(long baud){
	if(_sleepPin){
		pinMode(_sleepPin,OUTPUT);
		digitalWrite(_sleepPin,LOW);
	}
	i_begin(baud);
}
/*
uint8_t hex2int(uint8_t c){
	if(c<='9') return c-'0';
	else return 10+c-'A';
}
*/


uint8_t BayXBeeInterface::sendPayload(void){
	if(_sleepPin){
		digitalWrite(_sleepPin,LOW);
		delay(_wakeupTime);
	}
	_tx.setPayloadLength(_next);
	_tx.setAddress16(_destination);
	uint8_t r_value=0;
	send(_tx);
	if(_waitTimeForResponse>0){
		if (readPacket(_waitTimeForResponse)) {
			// got a response!
			// should be a znet tx status
			if (getResponse().getApiId() == TX_STATUS_RESPONSE) {
				getResponse().getTxStatusResponse(_txStatus);
				// get the delivery status, the fifth byte
				if (_txStatus.getStatus() == SUCCESS) r_value=0;
				else r_value=XBeeError_NoSuccess;
			}
		} else r_value=XBeeError_Timeout;
	}
	if(_sleepPin){
		digitalWrite(_sleepPin,HIGH);
	}
	return r_value;
}

#if ENABLE_RX
uint8_t BayXBeeInterface::readIntoPayload(int timeout){
	if (! readPacket(timeout))  return 2;
	// got something
    if (getResponse().getApiId() != RX_16_RESPONSE ) return 3;
    // got a rx16 packet
    if(getResponse().isError()) return 1;

    getResponse().getRx16Response(_rx);
    startFrame(_rx.getData(0));
    for(uint8_t i=1; i<_rx.getDataLength();i++){
		addToPayload(_rx.getData(i));
    }
    return 0;
}
#endif
