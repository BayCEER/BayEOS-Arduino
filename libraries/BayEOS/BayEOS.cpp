#include "BayEOS.h"

void BayEOS::startFrame(uint8_t type){
	_next=0;
	addToPayload(type);
}

void BayEOS::startDataFrame(uint8_t subtype){
	startFrame((uint8_t) BayEOS_DataFrame);
	addToPayload(subtype);
}

void BayEOS::startOriginFrame(const String &o){
	startFrame((uint8_t) BayEOS_OriginFrame);
	addToPayload((uint8_t) o.length());
	addToPayload(o);
}

void BayEOS::startDataFrameWithOrigin(uint8_t subtype,const String &o){
	startOriginFrame(o);
	addToPayload((uint8_t) BayEOS_DataFrame);
	addToPayload(subtype);
}

uint8_t BayEOS::addChannelValue(double v,uint8_t channel_number){
	return addChannelValue((float) v,channel_number);
}
uint8_t BayEOS::addChannelValue(long v,uint8_t channel_number){
	return addChannelValue((float) v,channel_number);
}
uint8_t BayEOS::addChannelValue(unsigned long v,uint8_t channel_number){
	return addChannelValue((float) v,channel_number);
}
uint8_t BayEOS::addChannelValue(int v,uint8_t channel_number){
	return addChannelValue((float) v,channel_number);
}
uint8_t BayEOS::addChannelValue(unsigned int v,uint8_t channel_number){
	return addChannelValue((float) v,channel_number);
}
uint8_t BayEOS::addChannelValue(uint8_t v,uint8_t channel_number){
	return addChannelValue((float) v,channel_number);
}
uint8_t BayEOS::addChannelValue(int8_t v,uint8_t channel_number){
	return addChannelValue((float) v,channel_number);
}
uint8_t BayEOS::addChannelValue(float v,uint8_t channel_number){
	uint8_t res=0;
	uint8_t offset=0;
	if(_payload[0]==BayEOS_OriginFrame){
		offset=_payload[1]+2;
	}
	if(_payload[offset]!=BayEOS_DataFrame) return 2;
	if(_payload[offset+1] & BayEOS_ChannelLabel) return 3;
	if( ( (_payload[offset+1] & 0xf0)==0x40) )
		addToPayload(channel_number); //channel number
	else if( ( ( (_payload[offset+1] & 0xf0)==0x0 ) & (_next==(2+offset)) ) )
		addToPayload(channel_number); //offset - only once

	switch(_payload[offset+1] & 0x0f){
		case 1:
			res=addToPayload((float) v);
			break;
		case 2:
			res=addToPayload((long) v);
			break;
		case 3:
			res=addToPayload((int) v);
			break;
		case 4:
			res=addToPayload((uint8_t) v);
			break;
	}
	if(res) return 0;
	else return 1;
}


uint8_t BayEOS::addChannelValue(double v,const char* channel_label){
	return addChannelValue((float) v,channel_label);
}
uint8_t BayEOS::addChannelValue(long v,const char* channel_label){
	return addChannelValue((float) v,channel_label);
}
uint8_t BayEOS::addChannelValue(unsigned long v,const char* channel_label){
	return addChannelValue((float) v,channel_label);
}
uint8_t BayEOS::addChannelValue(int v,const char* channel_label){
	return addChannelValue((float) v,channel_label);
}
uint8_t BayEOS::addChannelValue(unsigned int v,const char* channel_label){
	return addChannelValue((float) v,channel_label);
}
uint8_t BayEOS::addChannelValue(uint8_t v,const char* channel_label){
	return addChannelValue((float) v,channel_label);
}
uint8_t BayEOS::addChannelValue(int8_t v,const char* channel_label){
	return addChannelValue((float) v,channel_label);
}
uint8_t BayEOS::addChannelValue(float v,const char* channel_label){
	uint8_t res=0;
	uint8_t offset=0;
	if(_payload[0]==BayEOS_OriginFrame){
		offset=_payload[1]+2;
	}
	if(_payload[offset]!=BayEOS_DataFrame) return 2;
	if(!(_payload[offset+1] & BayEOS_ChannelLabel)) return 3;
	addToPayload((uint8_t) strlen(channel_label));
	addToPayload(channel_label);
	switch(_payload[offset+1] & 0x0f){
		case 1:
			res=addToPayload((float) v);
			break;
		case 2:
			res=addToPayload((long) v);
			break;
		case 3:
			res=addToPayload((int) v);
			break;
		case 4:
			res=addToPayload((uint8_t) v);
			break;
	}
	if(res) return 0;
	else return 1;
}

void BayEOS::startRoutedFrame(uint16_t sourceMyID,uint16_t sourcePANID,uint8_t rssi){
	if(rssi) startFrame((uint8_t) BayEOS_RoutedFrameRSSI);
	else startFrame((uint8_t) BayEOS_RoutedFrame);
	addToPayload(sourceMyID);
	addToPayload(sourcePANID);
	if(rssi) addToPayload(rssi);
}

void BayEOS::startDelayedFrame(unsigned long delay){
	startFrame((uint8_t) BayEOS_DelayedFrame);
	addToPayload(delay);
}

void BayEOS::startTimestampFrame(unsigned long timestamp){
	startFrame((uint8_t) BayEOS_TimestampFrame);
	addToPayload(timestamp);
}

void BayEOS::startCommand(uint8_t cmd_api){
	startFrame((uint8_t) BayEOS_Command);
	addToPayload(cmd_api);
}

void BayEOS::startCommandResponse(uint8_t cmd_api){
	startFrame((uint8_t) BayEOS_CommandResponse);
	addToPayload(cmd_api);
}

uint8_t BayEOS::addToPayload(const void* p,uint8_t length){
	uint8_t* b=(uint8_t *) p;
	for(int i=0;i<length;i++){
		_success=addToPayload(b[i]);
	}
	return _success;
}

uint8_t BayEOS::addToPayload(const uint8_t* c){
	while(*c){
		_success=addToPayload(*c);
		c++;
	}
	return _success;
}
uint8_t BayEOS::addToPayload(const String &s){
	for(uint8_t i=0;i<s.length();i++){
		_success=addToPayload((uint8_t) s[i]);
	}
	return _success;
}

uint8_t BayEOS::addToPayload(int w){
	return addToPayload(&w,2);
}

uint8_t BayEOS::addToPayload(uint16_t w){
	return addToPayload(&w,2);
}

uint8_t BayEOS::addToPayload(float f){
	return addToPayload(&f,4);
}

uint8_t BayEOS::addToPayload(unsigned long l){
	return addToPayload(&l,4);
}

uint8_t BayEOS::addToPayload(long l){
	return addToPayload(&l,4);
}

uint8_t BayEOS::sendError(const String &s){
	startFrame((uint8_t) BayEOS_ErrorMessage);
	addToPayload(s);
	return sendPayload();
}

uint8_t BayEOS::sendMessage(const String &s){
	startFrame((uint8_t) BayEOS_Message);
	addToPayload(s);
	return sendPayload();
}

uint8_t BayEOS::writeToBuffer(void){
	if((getPayloadLength()-getPacketLength())<5) return 0;
//	Serial.println("writeToBuffer");
//	Serial.println(_buffer->getTime());
	return _buffer->addPacket(_payload,_next);
}

uint8_t BayEOS::readBinaryFromBuffer(unsigned long pos){
	startFrame(BayEOS_BinaryFrame);
	addToPayload(pos);
	_next+=_buffer->readBinary(pos,getPayloadLength()-5,&_payload[_next]);
	return _next-5;
}
uint8_t BayEOS::readBinaryFromBuffer(unsigned long pos,unsigned long stop,unsigned long vpos){
	startFrame(BayEOS_BinaryFrame);
	addToPayload(vpos);
	_next+=_buffer->readBinary(pos,stop,getPayloadLength()-5,&_payload[_next]);
	return _next-5;
}

uint8_t BayEOS::readFromBuffer(void){
	if(! _buffer->available()) return 0;
	_buffer->initNextPacket();
	if(_buffer->rtc()){
		if(_buffer->_absoluteTime)
			startTimestampFrame(_buffer->packetMillis());
		else
			startDelayedFrame((_buffer->getTime()-_buffer->packetMillis())*1000);
	}
	else
	  startDelayedFrame(millis()-_buffer->packetMillis());

    if(getPayloadBytesLeft()<_buffer->packetLength()){
    	_buffer->next();
    	return 0;
    }
    _buffer->readPacket(&_payload[_next]);
    _next+=_buffer->packetLength();
    return _buffer->packetLength();
}

uint8_t BayEOS::sendOrBuffer(void){
	//Try to send when no failure or _skip_counter has reached next try...
	if(_failure_counter<2
			|| _skip_counter>=(uint8_t) (_failure_counter*_failure_counter)
			|| _skip_counter>=_max_skip){
		_skip_counter=0;
		if(! sendPayload()){
			//success -> reset _failure_counter to 0
			_failure_counter=0;
			return 0;
		} else {
			//no success
			_failure_counter++;
		}
	} else _skip_counter++;
	if(writeToBuffer()) return 0;
	return 1;
}

uint8_t BayEOS::sendFromBuffer(void){
	if(readFromBuffer()){
		if(_failure_counter<2 ||
				_skip_counter>=(uint8_t) (_failure_counter*_failure_counter) ||
				_skip_counter>=_max_skip){
			_skip_counter=0;
			if(! sendPayload()){
				//success -> reset _failure_counter to 0
				_failure_counter=0;
				_buffer->next();
				return 0;
			} else {
				//no success
				_failure_counter++;
				return 1;
			}
		} else _skip_counter++;
	}
	return 0;
}

