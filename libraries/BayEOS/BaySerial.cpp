#include "BaySerial.h"
uint8_t BaySerialInterface::readByte(bool escape){
	_read_timeout=false;
	uint8_t b=0;
	while((millis()-_start)<_current_timeout){
        if(i_available()){
        	b = read();
        	if (escape && b == ESCAPE) {
        		while(! i_available() && (millis()-_start)<_current_timeout){
        			delay(1);
        		}
        		if (i_available()) {
        			b = read();
        			return 0x20 ^ b;
        		} else {
        			_read_timeout=true;
        		}
        	}
        	return b;
        }
		delay(1);
	}
	_read_timeout=true;
	return b;

}


void BaySerialInterface::sendByte(uint8_t b, bool escape) {
	if (escape && (b == START_BYTE || b == ESCAPE || b == XON || b == XOFF || b=='\n' || b=='\r')) {
		write(ESCAPE);
		write(b ^ 0x20);
	} else {
		write(b);
	}

}

void BaySerialInterface::sendAck(uint8_t b){
	sendByte(START_BYTE,false);
	sendByte(0x1,true);
	sendByte(API_ACK,true);
	sendByte(b,true);
	sendByte(0xff-(b+API_ACK),true);
	//Serial.print("A");
	flush();
}

void BaySerialInterface::sendFrame(void){
	//Serial.print("F");
	sendByte(START_BYTE,false);
	sendByte(getPacketLength(),true);
	sendByte(API_DATA,true);
	_checksumTotal=0x1;
	for(uint8_t i=0;i<getPacketLength();i++){
		sendByte(_payload[i],true);
		_checksumTotal+=_payload[i];
	}
	sendByte((0xff-_checksumTotal),true);
	flush();
}

uint8_t BaySerialInterface::sendPayload(void){
	//Serial.print("S");
	if(_break) return TX_BREAK;
	if(_cts_pin){
		for(uint8_t i=0;i<3;i++){
			pinMode(_cts_pin,INPUT); //release CTS line for a short time
			if( ! digitalRead(_cts_pin)) return TX_BUSY;
			pinMode(_cts_pin,OUTPUT); //pull down CTS line
			delayMicroseconds(random(1000));
		}
		begin(_baud);
	}
	sendFrame();
	delay(1);
	uint8_t res=readPacket(API_ACK);
	if(res && _retries){
		for(uint8_t i=0;i<_retries;i++){
			delay(10);
			sendFrame();
			res=readPacket(API_ACK);
			if(! res) break;
		}
	}

	if(_cts_pin){
		end();
		pinMode(_cts_pin,INPUT);
	}
	if(! res && _ack!=TX_OK) res=1;
	return res;

}
uint8_t BaySerialInterface::readIntoPayload(int timeout) {
	return readPacket(API_DATA,timeout);
}

uint8_t BaySerialInterface::readPacket(uint8_t type,int timeout) {
	if(timeout) _current_timeout=timeout;
	else _current_timeout=_timeout;
	_start=millis();
	start:
	//Serial.print("P");
	//Serial.print(millis());
	_pos=0;
	uint8_t b=0;
	while(true){
        b = readByte(false);
    	if(_read_timeout) return 2;
        if(b == START_BYTE) break;
	}

	_length=readByte(true);
	if(_read_timeout) return 2;
	//Serial.print("P");
	//Serial.print(millis());

	//Serial.print("L");

	_api=readByte(true);
	_checksumTotal=_api;
	if(_read_timeout) return 2;
	//Serial.print("P");

	while(_pos<_length){
		b=readByte(true);
		if(_read_timeout) return 2;
		_checksumTotal+= b;
		if(_api==API_DATA) _payload[_pos] = b;
		else _ack=b;
		_pos++;
	}
	//Serial.print("P");
	//Serial.print(millis());

	_next=_pos;
	b=readByte(true);
	if(_read_timeout) return 2;
	_checksumTotal+= b;


	if(_api!=type){
		goto start;
	}
	// set break
	if(_api==API_ACK && _ack==TX_BREAK){
		_break=1;
		return TX_BREAK;
	}
	if(type==API_DATA && i_available()) goto start; //obviously old packet!
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

BaySerial::BaySerial(HardwareSerial &serial,int timeout,long baud,uint8_t cts_pin){
   BaySerialInterface::_timeout=timeout;
   BaySerialInterface::_baud=baud;
   BaySerialInterface::_cts_pin=cts_pin;
   _serial=&serial;
}

int BaySerial::available(void){
	return _serial->available();
}
int BaySerial::i_available(void){
	return _serial->available();
}
void BaySerial::begin(long baud){
	_baud=baud;
	_serial->begin(baud);
}
void BaySerial::flush(void){
	_serial->flush();
}
void BaySerial::end(void){
	_serial->end();
}
int BaySerial::read(void){
	return _serial->read();
}

size_t BaySerial::write(uint8_t c){
	return _serial->write(c);
}
BaySerialESP::BaySerialESP(HardwareSerial& serial, uint8_t ch_pd_pin, int timeout) : BaySerial(serial, timeout){
	_ch_pd_pin=ch_pd_pin;
}
uint8_t BaySerialESP::powerUp(uint8_t tries){
	if(! _ch_pd_pin) return 0;
	pinMode(_ch_pd_pin,OUTPUT);
	digitalWrite(_ch_pd_pin,HIGH);
	uint8_t res;
	while(tries){
		res=isReady();
		if(! res) return 0;
		delay(300);
		tries--;
	}
	return res;
}

void BaySerialESP::powerDown(){
	if(! _ch_pd_pin) return;
	pinMode(_ch_pd_pin,OUTPUT);
	digitalWrite(_ch_pd_pin,LOW);
}

uint8_t BaySerialESP::isReady(void){
	startCommand(BayEOS_RouterCommand);
	addToPayload((uint8_t) ROUTER_IS_READY);
	if(sendPayload()) return 2; //no ack
	if(readIntoPayload()) return 3; //no response
	if(getPayload(2)==ROUTER_IS_READY && getPayload(1)==BayEOS_RouterCommand) return getPayload(3);
	else return 4; //wrong response
}

uint8_t BaySerialESP::setName(char* name){
	startCommand(BayEOS_RouterCommand);
	addToPayload((uint8_t) ROUTER_SET_NAME);
	addToPayload(name);
	addToPayload((uint8_t) 0);
	if(sendPayload()) return 2; //no ack
	if(readIntoPayload()) return 3; //no response
	if(getPayload(2)==ROUTER_SET_NAME && getPayload(1)==BayEOS_RouterCommand) return getPayload(3);
	else return 4; //wrong response
}

uint8_t BaySerialESP::setConfig(char* value,uint8_t field){
	startCommand(BayEOS_RouterCommand);
	addToPayload((uint8_t) ROUTER_SET_CONFIG);
	addToPayload(field);
	addToPayload(value);
	addToPayload((uint8_t) 0);
	if(sendPayload()) return 2; //no ack
	if(readIntoPayload()) return 3; //no response
	if(getPayload(2)==ROUTER_SET_CONFIG && getPayload(1)==BayEOS_RouterCommand) return getPayload(3);
	else return 4; //wrong response
}



uint8_t BaySerialESP::sendMultiFromBuffer(uint16_t maxsize){
	if(! _buffer->available()) return 0;
	uint8_t res=isReady();
	if(res) return res+10;
	unsigned long read_pos=_buffer->readPos();

	while(_buffer->available() && (_buffer->readPos()-read_pos)<maxsize && ! res){
		res=sendFromBuffer();
	}

	if(res){
		_buffer->seekReadPointer(read_pos);
		return 20;
	}

	startCommand(BayEOS_RouterCommand);
    addToPayload((uint8_t) ROUTER_SEND);
	if(sendPayload()){
		_buffer->seekReadPointer(read_pos);
		return 22; //no ack
	}
	uint8_t tries=0;
	while(readIntoPayload()){
		tries++;
		if(tries>15){
			_buffer->seekReadPointer(read_pos);
			return 23; //no response
		}
	}
	if(getPayload(2)==ROUTER_SEND && getPayload(1)==BayEOS_RouterCommand)
		res=getPayload(3);
	else res=4;
	if(res ) _buffer->seekReadPointer(read_pos);
	return res;
}
