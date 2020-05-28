#include "BayEOS.h"

void BayEOS::startFrame(uint8_t type) {
	_next = 0;
	addToPayload(type);
}

void BayEOS::startFrame(void) {
	_next = 0;
}

void BayEOS::startDataFrame(uint8_t subtype, uint8_t checksum) {
	_next = 0;
	if (checksum)
		addToPayload((uint8_t) BayEOS_ChecksumFrame);
	addToPayload((uint8_t) BayEOS_DataFrame);
	addToPayload(subtype);
}

void BayEOS::startOriginFrame(const String &o, uint8_t routed) {
	if(routed) startFrame((uint8_t) BayEOS_RoutedOriginFrame);
	else startFrame((uint8_t) BayEOS_OriginFrame);
	addToPayload((uint8_t) o.length());
	addToPayload(o);
}

void BayEOS::startDataFrameWithOrigin(uint8_t subtype, const String &o,
		uint8_t checksum, uint8_t routed) {
	_next = 0;
	if (checksum)
		addToPayload((uint8_t) BayEOS_ChecksumFrame);
	if(routed) addToPayload((uint8_t) BayEOS_RoutedOriginFrame);
	else addToPayload((uint8_t) BayEOS_OriginFrame);
	addToPayload((uint8_t) o.length());
	addToPayload(o);
	addToPayload((uint8_t) BayEOS_DataFrame);
	addToPayload(subtype);
}

uint8_t BayEOS::addChannelValue(double v, uint8_t channel_number) {
	return addChannelValue((float) v, channel_number);
}
uint8_t BayEOS::addChannelValue(long v, uint8_t channel_number) {
	return addChannelValue((float) v, channel_number);
}
uint8_t BayEOS::addChannelValue(unsigned long v, uint8_t channel_number) {
	return addChannelValue((float) v, channel_number);
}
uint8_t BayEOS::addChannelValue(int v, uint8_t channel_number) {
	return addChannelValue((float) v, channel_number);
}
uint8_t BayEOS::addChannelValue(unsigned int v, uint8_t channel_number) {
	return addChannelValue((float) v, channel_number);
}
uint8_t BayEOS::addChannelValue(uint8_t v, uint8_t channel_number) {
	return addChannelValue((float) v, channel_number);
}
uint8_t BayEOS::addChannelValue(int8_t v, uint8_t channel_number) {
	return addChannelValue((float) v, channel_number);
}
uint8_t BayEOS::addChannelValue(float v, uint8_t channel_number) {
	uint8_t res = 0;
	uint8_t offset = 0;
	if (_payload[0] == BayEOS_ChecksumFrame) {
		offset = 1;
	}
	if (_payload[offset] == BayEOS_OriginFrame || _payload[offset] == BayEOS_RoutedOriginFrame) {
		offset += _payload[offset + 1] + 2;
	}
	if (_payload[offset] != BayEOS_DataFrame)
		return 2;
	if ((_payload[offset + 1] & BayEOS_OFFSETTYP_MASK) == BayEOS_ChannelLabel)
		return 3;
	if (((_payload[offset + 1] & BayEOS_OFFSETTYP_MASK) == BayEOS_ChannelNumber))
		addToPayload(channel_number); //channel number
	else if ((((_payload[offset + 1] & BayEOS_OFFSETTYP_MASK) == 0x0)
			& (_next == (2 + offset)))) {
		if (channel_number)
			channel_number--;
		addToPayload(channel_number); //offset - only once
	}
	switch (_payload[offset + 1] & 0x0f) {
	case 1:
		res = addToPayload((float) v);
		break;
	case 2:
		res = addToPayload((long) v);
		break;
	case 3:
		res = addToPayload((int) v);
		break;
	case 4:
		res = addToPayload((uint8_t) v);
		break;
	}
	if (res)
		return 0;
	else
		return 1;
}

uint8_t BayEOS::addChannelValue(double v, const char* channel_label) {
	return addChannelValue((float) v, channel_label);
}
uint8_t BayEOS::addChannelValue(long v, const char* channel_label) {
	return addChannelValue((float) v, channel_label);
}
uint8_t BayEOS::addChannelValue(unsigned long v, const char* channel_label) {
	return addChannelValue((float) v, channel_label);
}
uint8_t BayEOS::addChannelValue(int v, const char* channel_label) {
	return addChannelValue((float) v, channel_label);
}
uint8_t BayEOS::addChannelValue(unsigned int v, const char* channel_label) {
	return addChannelValue((float) v, channel_label);
}
uint8_t BayEOS::addChannelValue(uint8_t v, const char* channel_label) {
	return addChannelValue((float) v, channel_label);
}
uint8_t BayEOS::addChannelValue(int8_t v, const char* channel_label) {
	return addChannelValue((float) v, channel_label);
}
uint8_t BayEOS::addChannelValue(float v, const char* channel_label) {
	uint8_t res = 0;
	uint8_t offset = 0;
	if (_payload[0] == BayEOS_ChecksumFrame) {
		offset = 1;
	}
	if (_payload[offset] == BayEOS_OriginFrame || _payload[offset] == BayEOS_RoutedOriginFrame) {
		offset += _payload[offset + 1] + 2;
	}
	if (_payload[offset] != BayEOS_DataFrame)
		return 2;
	if ((_payload[offset + 1] & BayEOS_OFFSETTYP_MASK) != BayEOS_ChannelLabel)
		return 3;
	addToPayload((uint8_t) strlen(channel_label));
	addToPayload(channel_label);
	switch (_payload[offset + 1] & 0x0f) {
	case 1:
		res = addToPayload((float) v);
		break;
	case 2:
		res = addToPayload((long) v);
		break;
	case 3:
		res = addToPayload((int) v);
		break;
	case 4:
		res = addToPayload((uint8_t) v);
		break;
	}
	if (res)
		return 0;
	else
		return 1;
}

uint8_t BayEOS::addChecksum() {
	uint16_t checksum = 0;
	for (uint8_t i = 0; i < _next; i++) {
		checksum += _payload[i];
	}
	return addToPayload((uint16_t)(0xffff - checksum & 0xffff));
}
uint8_t BayEOS::validateChecksum() {
	uint8_t offset = 0;
	uint8_t has_checksum = 0;
	uint16_t checksum = 0;
	while (!has_checksum) {
		switch (_payload[offset]) {
		case BayEOS_RF24Frame:
			offset += 2;
			break;
		case BayEOS_RoutedFrameRSSI:
			offset += 6;
			break;
		case BayEOS_DelayedFrame:
		case BayEOS_DelayedSecondFrame:
		case BayEOS_TimestampFrame:
		case BayEOS_RoutedFrame:
			offset += 5;
			break;
		case BayEOS_OriginFrame:
		case BayEOS_RoutedOriginFrame:
			offset += 2+_payload[offset+1];
			break;
		case BayEOS_ChecksumFrame:
			has_checksum = 1;
			break;
		default:
			return 2;
		}
	}
	while (offset < getPacketLength() - 2) {
		checksum += _payload[offset];
		offset++;
	}
	checksum += *(uint16_t*) (_payload+offset);
	if(checksum == 0xffff) return 0;
	return 1;
}

void BayEOS::startRoutedFrame(uint16_t sourceMyID, uint16_t sourcePANID,
		uint8_t rssi) {
	if (rssi)
		startFrame((uint8_t) BayEOS_RoutedFrameRSSI);
	else
		startFrame((uint8_t) BayEOS_RoutedFrame);
	addToPayload(sourceMyID);
	addToPayload(sourcePANID);
	if (rssi)
		addToPayload(rssi);
}

void BayEOS::startDelayedFrame(unsigned long delay) {
	startFrame((uint8_t) BayEOS_DelayedFrame);
	addToPayload(delay);
}

void BayEOS::startDelayedSecondFrame(unsigned long delay) {
	startFrame((uint8_t) BayEOS_DelayedSecondFrame);
	addToPayload(delay);
}

void BayEOS::startTimestampFrame(unsigned long timestamp) {
	startFrame((uint8_t) BayEOS_TimestampFrame);
	addToPayload(timestamp);
}

void BayEOS::startRF24Frame(uint8_t pipe) {
	startFrame((uint8_t) BayEOS_RF24Frame);
	addToPayload(pipe);
}

void BayEOS::startCommand(uint8_t cmd_api) {
	startFrame((uint8_t) BayEOS_Command);
	addToPayload(cmd_api);
}

void BayEOS::startCommandResponse(uint8_t cmd_api) {
	startFrame((uint8_t) BayEOS_CommandResponse);
	addToPayload(cmd_api);
}

uint8_t BayEOS::addToPayload(const void* p, uint8_t length) {
	uint8_t* b = (uint8_t *) p;
	for (int i = 0; i < length; i++) {
		_success = addToPayload(b[i]);
	}
	return _success;
}

uint8_t BayEOS::addToPayload(const uint8_t* c) {
	while (*c) {
		_success = addToPayload(*c);
		c++;
	}
	return _success;
}
uint8_t BayEOS::addToPayload(const String &s) {
	for (uint8_t i = 0; i < s.length(); i++) {
		_success = addToPayload((uint8_t) s[i]);
	}
	return _success;
}

uint8_t BayEOS::addToPayload(int w) {
	return addToPayload(&w, 2);
}

uint8_t BayEOS::addToPayload(uint16_t w) {
	return addToPayload(&w, 2);
}

uint8_t BayEOS::addToPayload(float f) {
	return addToPayload(&f, 4);
}

uint8_t BayEOS::addToPayload(unsigned long l) {
	return addToPayload(&l, 4);
}

uint8_t BayEOS::addToPayload(long l) {
	return addToPayload(&l, 4);
}

uint8_t BayEOS::sendError(const String &s) {
	startFrame((uint8_t) BayEOS_ErrorMessage);
	addToPayload(s);
	return sendPayload();
}

uint8_t BayEOS::sendMessage(const String &s) {
	startFrame((uint8_t) BayEOS_Message);
	addToPayload(s);
	return sendPayload();
}

uint8_t BayEOS::createMessage(const String &s,uint8_t checksum, uint8_t frametype) {
	_next = 0;
	uint8_t ret;
	if (checksum)
		addToPayload((uint8_t) BayEOS_ChecksumFrame);
	addToPayload(frametype);
	ret=addToPayload(s);
	if (checksum)
		ret=addChecksum();
	return ret;
}

uint8_t BayEOS::writeToBuffer(void) {
	if ((getPayloadLength() - getPacketLength()) < 5)
		return 0;
//	Serial.println("writeToBuffer");
//	Serial.println(_buffer->getTime());
	return _buffer->addPacket(_payload, _next);
}

uint8_t BayEOS::readBinaryFromBuffer(unsigned long pos) {
	startFrame(BayEOS_BinaryFrame);
	addToPayload(pos);
	_next += _buffer->readBinary(pos, getPayloadLength() - 5, &_payload[_next]);
	return _next - 5;
}
uint8_t BayEOS::readBinaryFromBuffer(unsigned long pos, unsigned long stop,
		unsigned long vpos) {
	startFrame(BayEOS_BinaryFrame);
	addToPayload(vpos);
	_next += _buffer->readBinary(pos, stop, getPayloadLength() - 5,
			&_payload[_next]);
	return _next - 5;
}

uint8_t BayEOS::readFromBuffer(void) {
	while (_buffer->available()) {
		_buffer->initNextPacket();
		if(_buffer->packetLength()>_buffer->available()){
			//invalid packet!!
			_buffer->skip();
			return 0;
		}
		if (!_buffer->packetLength() || _buffer->packetLength()>BayEOS_MAX_PAYLOAD) {
			_buffer->next(); //skip empty or too long packet
			continue;
		}
		if (_buffer->rtc()) {
			switch(_buffer->_timeType){
			case RTC_RELATIVE_MILLIS:
				startDelayedFrame(
						(_buffer->getTime() - _buffer->packetMillis()) * 1000);
				break;
			case RTC_RELATIVE_SECONDS:
				startDelayedSecondFrame((_buffer->getTime() - _buffer->packetMillis()));
				break;
			case RTC_ABSOLUTE_SECONDS:
				startTimestampFrame(_buffer->packetMillis());
				break;

			}

		} else
			startDelayedFrame(millis() - _buffer->packetMillis());

		if (getPayloadBytesLeft() < _buffer->packetLength()) {
			_buffer->next(); //skip packet - now way to send this...
			continue;
		}
		_buffer->readPacket(&_payload[_next]);
		if(_payload[_next]>0x40){
			_buffer->next(); //skip invalid frame
			continue;
		}
		_next += _buffer->packetLength();
		return _buffer->packetLength();
	}
	return 0;
}

uint8_t BayEOS::sendOrBuffer(void) {
	//Try to send when no failure or _skip_counter has reached next try...
	if (_failure_counter < 2
			|| _skip_counter
					>= (uint16_t)(
							(uint16_t) _failure_counter
									* (uint16_t) _failure_counter) /* 2->4 skip, 3->9 skip ...*/
			|| _skip_counter >= _max_skip) {
		_skip_counter = 0;
		if (!sendPayload()) {
			//success -> reset _failure_counter to 0
			_failure_counter = 0;
			return 0;
		} else {
			//no success
			if (_failure_counter != 255)
				_failure_counter++;
		}
	} else
		_skip_counter++;
	if (writeToBuffer())
		return 0;
	return 1;
}

uint8_t BayEOS::sendFromBuffer(void) {
	if (readFromBuffer()) {
		if (_failure_counter < 2
				|| /* not too much failures */
				_skip_counter
						>= (uint16_t)(
								(uint16_t) _failure_counter
										* (uint16_t) _failure_counter) || /* 2->4 skip, 3->9 skip ...*/
				_skip_counter >= _max_skip) {
			_skip_counter = 0;
			if (!sendPayload()) {
				//success -> reset _failure_counter to 0
				_failure_counter = 0;
				_buffer->next();
				return 0;
			} else {
				//no success
				if (_failure_counter != 255)
					_failure_counter++;
				return 1;
			}
		} else {
			_skip_counter++;
			return 1;
		}
	}
	return 0;
}

uint8_t BayEOS::getPayloadBytesLeft(void) const{
	return (BayEOS_MAX_PAYLOAD-_next);
}

uint8_t BayEOS::getPayloadLength(void) const{
	return BayEOS_MAX_PAYLOAD;
}

uint8_t BayEOS::getPacketLength(void) const{
	return _next;
}

const uint8_t* BayEOS::getPayload(void) const{
	return _payload;
}

uint8_t BayEOS::getPayload(uint8_t index) const{
	return _payload[index];
}


