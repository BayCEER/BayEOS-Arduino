#include "BayRF24.h"

BayRF24::BayRF24(uint8_t _cepin, uint8_t _cspin, uint8_t powerdown = 1) :
		RF24(_cepin, _cspin) {
	_powerdown = powerdown;
}

uint8_t BayRF24::readIntoPayload(void){
	if(! RF24::available() && ! isAckPayloadAvailable() ) return 1;
	_next=RF24::getDynamicPayloadSize(); //store end pos
	RF24::read(BayEOS::_payload,_next); //read directly into _payload
	return 0;
}

uint8_t BayRF24::sendPayload(void) {
	if (_powerdown)
		powerUp();
	else
		stopListening();
	openWritingPipe (_pipe);
	uint8_t res;
	res = RF24::write(BayEOS::getPayload(), BayEOS::getPacketLength());
	uint8_t curr_pa = 0;
	while (!res && curr_pa < 4) {
		setPALevel((rf24_pa_dbm_e) curr_pa);
		delayMicroseconds(random(2000));
		res = RF24::write(BayEOS::getPayload(), BayEOS::getPacketLength());
		curr_pa++;
	}

	if (_powerdown)
		powerDown();
	else {
		txStandBy();
		startListening();
	}

	return !res;
}

void BayRF24::setTXAddr(uint64_t address){
	_pipe = address;

}
void BayRF24::setTXAddr(uint8_t* address){
	memcpy((void*)&_pipe,address,5);

}

void BayRF24::init(uint64_t address, uint8_t c = 0x71, rf24_pa_dbm_e pa_level =
		RF24_PA_HIGH, rf24_datarate_e rate = RF24_250KBPS) {
	_pipe = address;
	RF24::begin();
	setChannel(c);
	setPayloadSize(32);
	enableDynamicPayloads();
	enableAckPayload();               // Allow optional ack payloads
	setCRCLength (RF24_CRC_16);
	setDataRate(rate);
	setPALevel(pa_level);
	_pa_level = pa_level;
//changed 0.1.2 - as we normally have a storage on board
//User can call client.setRetries(15,15) after client.init
	setRetries(15, 8);
	setAutoAck(true);
	if (_powerdown)
		powerDown();
}

bool BayRF24::i_available(uint8_t* pipe_nr) {
	return RF24::available(pipe_nr);
}
