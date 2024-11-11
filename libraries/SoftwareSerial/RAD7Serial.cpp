#include "RAD7Serial.h"
#include <RTClib.h>

RAD7Serial::RAD7Serial(HardwareSerial &serial, BayEOS &client, uint8_t maxPayload) {
	_serial = &serial;
	_client = &client;
	_maxChannel = (maxPayload-13)/4;
}

uint8_t RAD7Serial::deleteData(void) {
	if(wait_for_ready()){
		return 1;
	}
	_serial->print("Data Delete 01Yes\r\n");
	readline();
	readline();
	readline();
	_buffer[14] = 0;
	if (strcmp("Data Delete 01", _buffer)) {
		_client->startFrame(BayEOS_ErrorMessage);
		_client->addToPayload("Delete: No Data Delete 01");
		_client->writeToBuffer();
	}
	//Delete OK

	//Adjust bit-Array to new numbers
	uint16_t v; //16 bit INT
	uint8_t i;
	for(i=12; i<124; i++){
		v=*(uint16_t *)(_saved+i);
		v>>=3; //shift three to right as we deleted 99 Samples
		// No. 100 moves to 1, 101 to 2 ...
		_saved[i-12]=(uint8_t) v;
	}
	for(i=112;i<125;i++){
		_saved[i]=0;
	}

	_rnumber-=99;
	if(_rnumber<0) _rnumber=0;
	wait_for_ready();


	//Renumber
	_serial->print("Data RenumberYes\r\n");
	readline();
	readline();
	readline();
	_buffer[13] = 0;
	if (strcmp("Data Renumber", _buffer)) {
		_client->startFrame(BayEOS_ErrorMessage);
		_client->addToPayload("Delete: No Data Renumber");
		_client->writeToBuffer();
	}
	return 0;
}

int RAD7Serial::readData(void) {
	if(wait_for_ready()){
		return -1;
	}

	_serial->print("Special ComAll\r\n");
	if (readline()) {
		_client->startFrame(BayEOS_ErrorMessage);
		_client->addToPayload("no response");
		_client->writeToBuffer();
		return -2;
	}
	readline();
	readline();
	if (strcmp("Special ComAll ", _buffer)) {
		_client->startFrame(BayEOS_ErrorMessage);
		_client->addToPayload("no Special ComAll");
		_client->writeToBuffer();
#if RAD7SERIAL_DEBUG
		Serial.println("no Special ComAll");
		Serial.println(_buffer);
#endif
		return -3;
	}
	readline();
	if (strcmp("Data transfer...", _buffer)) {
		_client->startFrame(BayEOS_ErrorMessage);
		_client->addToPayload("no Data transfer...");
		_client->writeToBuffer();
#if RAD7SERIAL_DEBUG
		Serial.println("no Data transfer");
		Serial.println(_buffer);
#endif
		return -4;
	}

	while (!readline()) {
		char* p = _buffer;
		//Finish reading if line is to short. This is probably the last line, which is empty
		if (strlen(p) < 20)
			break;
		//save the run number index
		_rnumber = atoi(p);
		//continue if the run number is already in the buffer
		if (_saved[_rnumber / 8] & (1 << (_rnumber % 8)))
			continue;
		//move ahead parsing the line
		p += 4;
		int y = atoi(p);
		if (y < 16) {
			_client->startFrame(BayEOS_ErrorMessage);
			_client->addToPayload("wrong date");
			_client->writeToBuffer();
			return -4;
		}
		_saved[_rnumber / 8] |= (1 << (_rnumber % 8));
		p += 3;
		uint8_t m = atoi(p);
		p += 3;
		uint8_t d = atoi(p);
		p += 3;
		uint8_t h = atoi(p);
		p += 3;
		uint8_t mi = atoi(p);
		p += 3;
		DateTime t(2000 + y, m, d, h, mi);

		uint8_t count = 0;
		uint8_t offset = 0;
		_client->startTimestampFrame(t.get() - 3600);
		_client->addToPayload((uint8_t) 0x1);
		_client->addToPayload((uint8_t) 0x1);
		_client->addToPayload(offset); //DateFrame!
		while (true) {
			float f = atof(p);
			_client->addToPayload(f);
			count++;
			if (count >= _maxChannel) {
				_client->writeToBuffer();
				offset += count;
				count = 0;
				_client->startTimestampFrame(t.get() - 3600);
				_client->addToPayload((uint8_t) 0x1);
				_client->addToPayload((uint8_t) 0x1);
				_client->addToPayload(offset); //DateFrame!

			}
			while (*p != ',' && *p)
				p++;
			if (!*p)
				break;
			p++;
		}
		if (count)
			_client->writeToBuffer();

	}
	return _rnumber;
}

uint8_t RAD7Serial::readline(unsigned long timeout, uint8_t l) {
	uint8_t c = 0;
	unsigned long start = millis();
	int b;
	while (true) {
		while (!_serial->available()) {
			if ((millis() - start) > timeout)
				return 1;
		}
		b = _serial->read();
		if (b == -1)
			return 2;
		_buffer[c] = b;
#if RAD7SERIAL_DEBUG
		Serial.write((uint8_t) b);
#endif
		if (b == '>' && c == 0)
			return 4;
		if (b == '\r')
			_buffer[c] = 0;
		if (b == '\n') {
			_buffer[c] = 0;
			return 0;
		}
		c++;
		if (c > (l + 1))
			return 3;
	}

}

uint8_t RAD7Serial::wait_for_ready(void) {
	uint8_t res = 0;
	while ((res = readline()) == 0) {

	}
	if (res == 4)
		return 0;
	unsigned long start = millis();
	int b;
	b = _serial->read();
	if (b == '>')
		return 0;
	_serial->write(0x3);
	b=-2;
	while ((millis() - start) > 5000) {
		if(!_serial->available()) {
			b = _serial->read();
			if (b == -1)
				break;
#if RAD7SERIAL_DEBUG
		Serial.write(b);
#endif
		if (b == '>')
			return 0;
		}
	}
	_client->startFrame(BayEOS_ErrorMessage);
	_client->addToPayload("Not ready!");
	_client->writeToBuffer();
	return (-1)*b;


}
