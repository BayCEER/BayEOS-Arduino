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

void BayEOSLogger::run(void) {
	//This will only write Data, when _sampling_interval is reached...
	logData();

	//live data when in Live
	liveData(1000);

	handleCommand();

	//Send data stored on SD
	//Just one frame each time...
	sendData();

	//Send binary dump of SD-file
	//Just one part each time
	sendBinaryDump();
}

void BayEOSLogger::sendBinaryDump(void) {
	if(_mode!=LOGGER_MODE_DUMP) return;
	if(_bufferwrap && (_long1==_buffer->endPos())){
		_bufferwrap=0;
		_long1=0;
	}
	uint8_t read=_client->readBinaryFromBuffer(_long1,_long2,_long3);
	_long1+=read;
	_long3+=read;
	if(_long1==_long2) _mode=0;
	if(_client->sendPayload()){
		//send error!
		_mode=0;
	}
}

void BayEOSLogger::sendData(void) {
	if(_mode!=LOGGER_MODE_DATA) return;
	if (_long2 > _buffer->readPos()){
		if(_client->sendFromBuffer()){
			//send error!
			_mode=0;
			readFromEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);
			_buffer->seekReadPointer(_long1);
		}
	} else _mode = 0;
}

void BayEOSLogger::logData(void) {
	if(_logging_disabled) return;
	if(! (_client->getPayload(0)==BayEOS_DataFrame ||
	    _client->getPayload(0)==BayEOS_Message ||
	    _client->getPayload(0)==BayEOS_ErrorMessage) ) return;
	if(_rtc->now().get()==_last_measurement) return;
	//Log data on even timestamps or on delay...
	if((_rtc->now().get()  % _sampling_int)==0 || (_rtc->now().get() - _last_measurement) > _sampling_int){
		_last_measurement =_rtc->now().get();
		//Store frame to buffer
		//will use RTC-time as timestamp
		_client->writeToBuffer();
		if(_buffer->_framesDiscarded){ //Save new read pos to eeprom - Old might not be valid any more!!
			unsigned long rpos = _buffer->readPos();
			writeToEEPROM((uint8_t*) &rpos, 4, EEPROM_READ_POS_OFFSET);
			_buffer->_framesDiscarded=0;
		}
		_logged_flag=1;
	}
}

void BayEOSLogger::liveData(uint16_t wait) {
	if (_mode!=LOGGER_MODE_LIVE)
		return;
	if(_client->sendPayload())
		_mode=0;

	delay(wait);
}

void BayEOSLogger::init(BayEOS& client, BayEOSBuffer& buffer, RTC& rtc, uint16_t min_sampling_int) {
	_rtc = &rtc;
	_buffer = &buffer;
	_client = &client;
	_min_sampling_int=min_sampling_int;

	readFromEEPROM((uint8_t*) &_sampling_int, 2, EEPROM_SAMPLING_INT_OFFSET);
	if (_sampling_int == 0xffff) {
		_sampling_int = _min_sampling_int;
		writeToEEPROM((uint8_t*) &_sampling_int, 2, EEPROM_SAMPLING_INT_OFFSET);
	}
	readFromEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);
	if (_long1 > _buffer->writePos())
		_long1 = _buffer->writePos();

	_buffer->seekReadPointer(_long1);
	writeToEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);
	_logging_disabled=0;
}

void BayEOSLogger::handleCommand(void) {
	uint8_t i;
	if (!_client->available() || _client->readIntoPayload() ||
	_client->getPayload(0)!=BayEOS_Command) return;

	//got command
	//Serial.println("Got CMD");
	uint8_t cmd_api = _client->getPayload(1);
	uint8_t cmd_response_api = cmd_api;


	//This is for backward compatibility with older LoggerReaders
	if(cmd_api==BayEOS_StopData){
		_mode = 0;
		cmd_api = BayEOS_BufferCommand;
	}

	switch (cmd_api) {
	case BayEOS_StartBinaryDump:
		_mode=LOGGER_MODE_DUMP;
		if(_client->getPacketLength()>2){
			for (i = 0; i < 4; i++) {
				*(((uint8_t*) &_long1) + i) = _client->getPayload(i + 2);
			}
		} else _long1=0;

		if(_client->getPacketLength()>6){
			for (i = 0; i < 4; i++) {
				*(((uint8_t*) &_long2) + i) = _client->getPayload(i + 6);
			}

		} else if(_client->getPacketLength()<3)
			_long2 = _buffer->endPos();
		else _long2 = _buffer->writePos();
		if(_buffer->available() && _long1>=_long2) _bufferwrap=1;
		else _bufferwrap=0;
		_long3=0;
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

	case BayEOS_SetSamplingInt:
		for (i = 0; i < 2; i++) {
			*(((uint8_t*) &_sampling_int) + i) = _client->getPayload(i + 2);
		}
		if(_sampling_int<_min_sampling_int) _sampling_int=_min_sampling_int;
		writeToEEPROM((uint8_t*) &_sampling_int, 2, EEPROM_SAMPLING_INT_OFFSET);
		cmd_response_api = BayEOS_GetSamplingInt;
		break;

	case BayEOS_SetTime:
		for (i = 0; i < 4; i++) {
			*(((uint8_t*) &_long1) + i) = _client->getPayload(i + 2);
		}
		_rtc->adjust(DateTime(_long1));
		cmd_response_api = BayEOS_GetTime;
		break;
	case BayEOS_Seek:
		for (i = 0; i < 4; i++) {
			*(((uint8_t*) &_long1) + i) = _client->getPayload(i + 2);
		}
		_buffer->seekReadPointer(0);
		while(_buffer->available()){
			_buffer->initNextPacket();
			if(_buffer->packetMillis()>_long1) break;
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

	case BayEOS_StartData:
		_mode = LOGGER_MODE_DATA;
		_long2 = _buffer->writePos();
		switch (_client->getPayload(2)) {
		case 1: /* seek read-Pointer to start */
			_buffer->seekReadPointer(0);
			break;
		}
		break;
	case BayEOS_BufferCommand:
		if (_client->getPayload(2) < 5) {
			switch (_client->getPayload(2)) {
			case 1: /* reset Buffer */
				_buffer->reset();
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
				_buffer->set(_buffer->writePos());
				_buffer->seekReadPointer(_long2);
				break;
			}
			_long1 = _buffer->readPos();
			/* store current read poiter pos to eeprom */
			writeToEEPROM((uint8_t*) &_long1, 4, EEPROM_READ_POS_OFFSET);
			_long1 = _buffer->available(); //will return available bytes
		} else if (_client->getPayload(2) == 5)
			_long1 = _buffer->readPos();//will return read pos
		else if (_client->getPayload(2) == 6)
			_long1 = _buffer->writePos();//will return write pos
		break;

	}

	//This is for backward compatibility with older LoggerReaders
	if(cmd_response_api==BayEOS_StopData){
		cmd_response_api=cmd_api;
		cmd_api=BayEOS_StopData;
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
	case BayEOS_GetSamplingInt:
		_client->addToPayload(_sampling_int);
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
		_client->addToPayload((_bufferwrap?(_buffer->endPos()-_long1+_long2):(_long2-_long1)));
		break;
	case BayEOS_StartData:
		_client->addToPayload(
				(unsigned long) (_buffer->writePos() - _buffer->readPos()));
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

