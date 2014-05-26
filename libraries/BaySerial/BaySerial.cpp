#include "BaySerial.h"
BaySerial::BaySerial(HardwareSerial &serial,int timeout):HardwareSerial(serial){
	_timeout=timeout;
}


uint8_t BaySerial::readByte(int timeout,bool escape){
	_read_timeout=false;
	uint8_t b=0;
	while(timeout>0){
        if(available()){
        	b = read();
        	if (escape && b == ESCAPE) {
        		if (available()) {
        			b = read();
        			b= 0x20 ^ b;
        		} else {
				// escape byte.  next byte will be
        			_escape = true;
        			continue;
        		}
        	}
        	if (_escape == true) {
        		b = 0x20 ^ b;
        		_escape = false;
        	}
        	return b;
        }
 		timeout--;
		delay(1);
	}
	_read_timeout=true;
	return b;

}

void BaySerial::begin(long baud){
	HardwareSerial::begin(baud);
}

void BaySerial::sendByte(uint8_t b, bool escape) {

	if (escape && (b == START_BYTE || b == ESCAPE || b == XON || b == XOFF || b=='\n' || b=='\r')) {
		write(ESCAPE);
		write(b ^ 0x20);
	} else {
		write(b);
	}
}

void BaySerial::sendAck(uint8_t b){
	sendByte(START_BYTE,false);
	sendByte(0x1,true);
	sendByte(0x2,true);
	sendByte(b,true);
//	sendByte(b+0x2,true); Ursprüngliche Version !!! FALSCH!!!
	sendByte(0xff-(b+0x2),true); //RICHTIG!
}

void BaySerial::sendFrame(void){
	sendByte(START_BYTE,false);
	sendByte(getPacketLength(),true);
	sendByte(0x1,true);
	_checksumTotal=0x1;
	for(uint8_t i=0;i<getPacketLength();i++){
		sendByte(_payload[i],true);
		_checksumTotal+=_payload[i];
	}
	sendByte((0xff-_checksumTotal),true);

}

uint8_t BaySerial::sendPayload(void){
	if(_break) return TX_BREAK;
	sendFrame();
	uint8_t res=0;
	if(res=readPacket(API_ACK)) return res;
	else if(_ack==TX_OK) return 0;
	else return 1;

}
uint8_t BaySerial::readIntoPayload(int timeout) {
	_read_timeout=timeout;
	return readPacket(API_DATA);
}

uint8_t BaySerial::readPacket(uint8_t type) {
	_pos=0;
	uint8_t b=0;
	while(true){
        b = readByte(_timeout,false);
    	if(_read_timeout) return 2;
        if(b == START_BYTE) break;
	}

	_length=readByte(_timeout,true);
	if(_read_timeout) return 2;

	_api=readByte(_timeout,true);
	if(_api!=type){
	   return readPacket(type);
	}
	_checksumTotal=_api;
	if(_read_timeout) return 2;



	while(_pos<_length){
		b=readByte(_timeout,true);
		if(_read_timeout) return 2;
		_checksumTotal+= b;
		if(_api==API_DATA) _payload[_pos] = b;
		else _ack=b;
		_pos++;
	}

	_next=_pos;
	b=readByte(_timeout,true);
	if(_read_timeout) return 2;
	_checksumTotal+= b;

	// set break
	if(_api==API_ACK && _ack==TX_BREAK){
		_break=1;
		return TX_BREAK;
	}
	// reset break when there is data
	if(_api==API_DATA && _break) _break=0;
	// verify checksum
	if ((_checksumTotal & 0xff) == 0xff) {
		if(_api==API_DATA) sendAck(TX_OK);
		return 0;
	} else {
		if(_api==API_DATA) sendAck(TX_CHECKSUM_FAILED);
		// checksum failed
		return 1;
	}
}
