#include "BayEOSLogger.h"
void writeToEEPROM(uint8_t* p, uint8_t length, int offset) {
	for (uint8_t i = 0; i < length; i++) {
		EEPROM.write(offset + i, *(p + i));
	}
}

void readFromEEPROM(uint8_t* p, uint8_t length, int offset) {
	for (uint8_t i = 0; i < length; i++) {
		*(p + i) = EEPROM.read(offset + i);
	}
}

void BayEOSLogger::setChannelMap(char* map) {
	_channel_map = map;
}

void BayEOSLogger::setUnitMap(char* map) {
	_unit_map = map;
}

void BayEOSLogger::run(bool connected) {
	//This will only write Data, when _sampling_interval is reached...
	logData();
	if (connected) {
		//live data when in Live
		liveData(1000);

		handleCommand();

		//Send binary dump of SD-file
		//Just one part each time
		sendBinaryDump();
	}
}

void BayEOSLogger::sendBinaryDump(void) {
	if (_mode != LOGGER_MODE_DUMP)
		return;
	uint8_t read = _client->readBinaryFromBuffer(_long1, _long2, _long3);
	_long1 += read;
	_long3 += read;
	//Wrap around!
	if (_long1 >= _buffer->length())
		_long1 -= _buffer->length();

	if (_long1 == _long2)
		_mode = 0;
	uint8_t e_count = 0;
	while (_client->sendPayload() && e_count < 3) {
		delay(50);
		//send error!
		e_count++;
	}
	if (e_count >= 3)
		_mode = 0;
}

void BayEOSLogger::logData(void) {
	if (_logging_disabled)
		return;
	if (_rtc->now().get() < _next_log)
		return;
	if (!(_client->getPayload(0) == BayEOS_DataFrame
			|| _client->getPayload(0) == BayEOS_Message
			|| _client->getPayload(0) == BayEOS_ErrorMessage))
		return;
	if(_framesize>=0){
		if(_framesize){
			if(_framesize!=(_client->getPacketLength()+5)) _framesize=-1; //different frame length
		} else {
			unsigned long l=_buffer->writePos()-_buffer->endPos();
			if(l>_buffer->length()) l+=_buffer->length();
			if(l%_client->getPacketLength()==0) _framesize=_client->getPacketLength()+5;
			else _framesize=-1;
		}
	}
	_next_log = (_rtc->now().get() / _sampling_int) * _sampling_int + _sampling_int;
	//Store frame to buffer
	//will use RTC-time as timestamp
	_client->writeToBuffer();
	if (_buffer->_framesDiscarded) { //Save new read pos to eeprom - Old might not be valid any more!!
		unsigned long rpos = _buffer->readPos();
		writeToEEPROM((uint8_t*) &rpos, 4, EEPROM_READ_POS_OFFSET);
		_buffer->_framesDiscarded = 0;
	}
	_logged_flag = 1;

}

void BayEOSLogger::liveData(uint16_t wait) {
	if (_mode != LOGGER_MODE_LIVE)
		return;
	if (_client->sendPayload())
		_mode = 0;
	while (wait) {
		if (_client->available()) {
			handleCommand();
			return;
		}
		delay(1);
		wait--;
	}
}

void BayEOSLogger::setClient(BayEOS& client) {
	_client = &client;
}


void BayEOSLogger::setResetCallback(reset_callback_function callback){
	_reset_callback=callback;
}

void BayEOSLogger::init(BayEOS& client, BayEOSBuffer& buffer, RTC& rtc,
		uint16_t min_sampling_int, uint16_t bat_warning) {
	_rtc = &rtc;
	_buffer = &buffer;
	_client = &client;
	_min_sampling_int = min_sampling_int;
	_bat_warning = bat_warning;

	readFromEEPROM((uint8_t*) &_sampling_int, 2, EEPROM_SAMPLING_INT_OFFSET);
	if (_sampling_int == 0xffff) {
		_sampling_int = _min_sampling_int;
		writeToEEPROM((uint8_t*) &_sampling_int, 2, EEPROM_SAMPLING_INT_OFFSET);
	}
	readFromEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);

	if (_long1 > _buffer->writePos() && _long1 < _buffer->endPos()) //invalid region
		_long1 = _buffer->writePos();
	if (_long1 >= _buffer->length())
		_long1 = _buffer->endPos(); //outside buffer end
	if (_buffer->endPos() == 0 && _buffer->readPos() == 0)
		_long1 = 0; //empty buffer
	_buffer->seekReadPointer(_long1);
	writeToEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);
	_logging_disabled = 0;
	_next_log = (_rtc->now().get() / _sampling_int) * _sampling_int + _sampling_int;
}

void BayEOSLogger::handleCommand(void) {
	uint8_t i;
	if (!_client->available() || _client->readIntoPayload() ||
	_client->getPayload(0)!=BayEOS_Command)
		return;

	//got command
	//Serial.println("Got CMD");
	uint8_t cmd_api = _client->getPayload(1);
	uint8_t cmd_response_api = cmd_api;

	/* DISABLED 1.4
	 //This is for backward compatibility with older LoggerReaders
	 if(cmd_api==BayEOS_StopData){
	 _mode = 0;
	 cmd_api = BayEOS_BufferCommand;
	 }
	 */
	switch (cmd_api) {
	case BayEOS_StartBinaryDump:
		_mode = LOGGER_MODE_DUMP;
		_long1 = _buffer->endPos();
		_long2 = _buffer->writePos();

		if (_client->getPacketLength() > 2) { //start
			for (i = 0; i < 4; i++) {
				*(((uint8_t*) &_long1) + i) = _client->getPayload(i + 2);
			}
		}
		if (_client->getPacketLength() > 6) { //end
			for (i = 0; i < 4; i++) {
				*(((uint8_t*) &_long2) + i) = _client->getPayload(i + 6);
			}

		}

		//if(_buffer->available() && _long1>=_long2) _bufferwrap=1;
		//else _bufferwrap=0;
		_long3 = 0;
		/*
		 Serial.print("BD\t");
		 Serial.print(_long1);
		 Serial.print("\t");
		 Serial.print(_long2);
		 Serial.print("\t");
		 Serial.println((_bufferwrap?(_buffer->endPos()-_long1+_long2):(_long2-_long1)));
		 */
		break;
	case BayEOS_SetName:
		EEPROM.write(EEPROM_NAME_OFFSET, EEPROM_NAME_STARTBYTE);
		for (i = 2; i < _client->getPacketLength(); i++) {
			EEPROM.write(EEPROM_NAME_OFFSET + i - 1, _client->getPayload(i));
		}
		EEPROM.write(EEPROM_NAME_OFFSET + i - 1, 0);
		cmd_response_api = BayEOS_GetName;
		break;

	case BayEOS_SetLoggingDisabled:
		_logging_disabled = _client->getPayload(2);
		cmd_response_api = BayEOS_GetLoggingDisabled;
		break;
	case BayEOS_SetSamplingInt:
		for (i = 0; i < 2; i++) {
			*(((uint8_t*) &_sampling_int) + i) = _client->getPayload(i + 2);
		}
		if (_sampling_int < _min_sampling_int)
			_sampling_int = _min_sampling_int;
		writeToEEPROM((uint8_t*) &_sampling_int, 2, EEPROM_SAMPLING_INT_OFFSET);
		cmd_response_api = BayEOS_GetSamplingInt;
		break;

	case BayEOS_SetTime:
		for (i = 0; i < 4; i++) {
			*(((uint8_t*) &_long1) + i) = _client->getPayload(i + 2);
		}
		_rtc->adjust(DateTime(_long1));
		_logging_disabled = false;
		cmd_response_api = BayEOS_GetTime;
		break;
	case BayEOS_Seek:
		for (i = 0; i < 4; i++) {
			*(((uint8_t*) &_long1) + i) = _client->getPayload(i + 2);
		}
		_buffer->seekReadPointer(_buffer->endPos());
		while (_buffer->available()) {
			_buffer->initNextPacket();
			if (_buffer->packetMillis() > _long1)
				break;
			_buffer->next();
		}
		cmd_response_api = BayEOS_TimeOfNextFrame;
		break;
	case BayEOS_StartLiveData:
		_mode = LOGGER_MODE_LIVE;
		break;
	case BayEOS_ModeStop:
		_mode = 0;
		break;
		/* DISABLED in 1.4
		 case BayEOS_StartData:
		 _mode = LOGGER_MODE_DATA;
		 _long2 = _buffer->writePos();
		 switch (_client->getPayload(2)) {
		 case 1: // seek read-Pointer to start
		 _buffer->seekReadPointer(_buffer->getEnd());
		 break;
		 }
		 break;
		 */
	case BayEOS_BufferCommand:
		if (_client->getPayload(2) < 5) {
			switch (_client->getPayload(2)) {
			case 1: /* reset Buffer */
				_buffer->reset();
				_framesize=0;
				if(_reset_callback) (*_reset_callback)();
				break;
			case 2: /* reset read pointer to last read out position */
				readFromEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);
				_buffer->seekReadPointer(_long1);
				break;
			case 3: /* set read pointer to current write pos */
				_buffer->seekReadPointer(_buffer->writePos());
				break;
			case 4: /* set to _long2 of last binary dump
			 please pay attention to do not call after [BinaryDump][start pos][end pos]
			 this may result in illegal positions
			 */
				//_buffer->set(_buffer->writePos()); ??
				_buffer->seekReadPointer(_long2);
				break;
			}
			_long1 = _buffer->readPos();
			/* store current read poiter pos to eeprom */
			writeToEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);
			_long1 = _buffer->available(); //will return available bytes
		} else {
			switch (_client->getPayload(2)) {
			case 5:
				_long1 = _buffer->readPos(); //will return read pos
				break;
			case 6:
				_long1 = _buffer->writePos(); //will return write pos
				break;
			case 7:
				_long1 = _buffer->endPos(); //will return end pos
				break;
			case 8:
				_long1 = _buffer->length(); //will return available bytes for new
				break;

			}

		}
		break;

	}
	//Command Response
	_client->startCommandResponse(cmd_api);
	switch (cmd_response_api) {
	case BayEOS_GetVersion:
		_client->addToPayload(BayEOS_VERSION);
		break;
	case BayEOS_BufferCommand:
		_client->addToPayload(_long1);
		break;
	case BayEOS_BufferInfo:
		_client->addToPayload(_buffer->readPos());
		_client->addToPayload(_buffer->writePos());
		_client->addToPayload(_buffer->endPos());
		_client->addToPayload(_buffer->length());
		_client->addToPayload((uint8_t)_framesize);
		_client->addToPayload(_sampling_int);
		break;
	case BayEOS_GetSamplingInt:
		_client->addToPayload(_sampling_int);
		break;
	case BayEOS_GetLoggingDisabled:
		_client->addToPayload(_logging_disabled);
		break;
	case BayEOS_GetBatStatus:
		_client->addToPayload(_bat);
		_client->addToPayload(_bat_warning);
		break;
	case BayEOS_GetName:
		i = 1;
		uint8_t n;
		if (EEPROM.read(EEPROM_NAME_OFFSET) == EEPROM_NAME_STARTBYTE) {
			while (n = EEPROM.read(EEPROM_NAME_OFFSET + i)) {
				_client->addToPayload(n);
				i++;
			}
		}
		break;
	case BayEOS_GetTime:
		_client->addToPayload((unsigned long) _rtc->now().get());
		break;
	case BayEOS_StartBinaryDump:
		_client->addToPayload(
				(_long1 > _long2 ?
						(_buffer->length() - _long1 + _long2) :
						(_long2 - _long1)));
		break;
	case BayEOS_GetChannelMap:
		if (_channel_map)
			_client->addToPayload(_channel_map);
		break;
	case BayEOS_GetUnitMap:
		if (_unit_map)
			_client->addToPayload(_unit_map);
		break;
	case BayEOS_TimeOfNextFrame:
		if (_client->readFromBuffer()) {
			for (i = 0; i < 4; i++) {
				*((uint8_t*) &_long1 + i) = _client->getPayload(i + 1);
			}
			_client->startCommandResponse(cmd_api);
			_client->addToPayload(_long1);
		} else {
			_client->startCommandResponse(cmd_api);
			_client->addToPayload((unsigned long) 0xffffffff);
		}
		break;
	}
	_client->sendPayload();

}

