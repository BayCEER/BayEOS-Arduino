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
uint16_t BayXBeeInterface::getPANID(void){
	uint8_t cmd[]={'I','D'};
	AtCommandRequest atRequest = AtCommandRequest(cmd);
	AtCommandResponse atResponse = AtCommandResponse();
	send(atRequest);
	if (readPacket(5000)) {
	    // got a response!
	    if (getResponse().getApiId() == AT_COMMAND_RESPONSE) {
	      getResponse().getAtCommandResponse(atResponse);

	      if (atResponse.isOk()) {
	    	  return ((uint16_t) atResponse.getValue()[0]<<8)+atResponse.getValue()[1];
	      }
	    }
	}
	return 0;
}

uint8_t BayXBeeInterface::parseRX16(BayEOS& client,int rx_panid){
	Rx16Response rx16 = Rx16Response();
	Rx16IoSampleResponse rx16io = Rx16IoSampleResponse();
	if (getResponse().isError())
		return 1; //Error
	if (getResponse().getApiId() == RX_16_IO_RESPONSE) {
		getResponse().getRx16IoSampleResponse(rx16io);
		client.startRoutedFrame((int) rx16io.getRemoteAddress16(), rx_panid,
				(uint8_t) rx16io.getRssi());
		client.addToPayload((uint8_t) 0x1); //DataFrame
		client.addToPayload((uint8_t) 0x43); //int16 + Channel Offset
		for (uint8_t i = 0; i <= 5; i++) { //Analog Samples
			if (rx16io.isAnalogEnabled(i)) {
				client.addToPayload(i); //offset
				client.addToPayload(rx16io.getAnalog(i, 0));
			}
		}
		for (uint8_t i = 0; i <= 8; i++) { //Digital Samples
			if (rx16io.isDigitalEnabled(i)) {
				client.addToPayload(i); //offset
				client.addToPayload((int) (rx16io.isDigitalOn(i, 0) ? 1 : 0));
			}
		}
		return 0;
	}

	if (getResponse().getApiId() == RX_16_RESPONSE) {
		// got a rx16 packet
		getResponse().getRx16Response(rx16);
		client.startRoutedFrame((int) rx16.getRemoteAddress16(), rx_panid,
				(uint8_t) rx16.getRssi());
		for (uint8_t i = 0; i < rx16.getDataLength(); i++) {
			client.addToPayload(rx16.getData(i));
		}
		return 0;
	}

	return 2;

}

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
