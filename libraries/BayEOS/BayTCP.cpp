#include "BayTCP.h"
const char* const BayTCPInterface::_urlencodedChars = "$&+,/:;=?@ <>#%{}|~[]`";

const uint8_t* BayTCPInterface::parseMAC(const char* str) {
	uint8_t offset = 0;
	uint8_t delim = 0;
	while (str[offset]) {
		if (str[offset] == '-') {
			delim = 1;
			break;
		}
		offset++;
	}
	if (!delim) {
		offset = 0;
		while (str[offset]) {
			if (str[offset] == ':') {
				delim = 1;
				break;
			}
			offset++;
		}

	}
	offset = 0;
	for (uint8_t i = 0; i < 6; i++) {
		_mac_ip[i] = (hex2int(str[offset]) << 4) + hex2int(str[offset + 1]);
		//	res[i]=0;
		offset += (2 + delim);
	}

	return _mac_ip;

}

uint8_t BayTCPInterface::hex2int(const char c) {
	if ((c >= '0') & (c <= '9'))
		return c - '0';
	if ((c >= 'A') & (c <= 'F'))
		return c - 'A' + 10;
	if ((c >= 'a') & (c <= 'f'))
		return c - 'a' + 10;
	return 0;
}

const uint8_t* BayTCPInterface::parseIP(const char* str) {
	uint8_t offset = 0;
	for (uint8_t i = 0; i < 4; i++) {
		_mac_ip[i] = atoi(str + offset);
		while (str[offset] && str[offset] != '.')
			offset++;
		offset++;
	}
	return _mac_ip;
}

void BayTCPInterface::printPGM(const char *str) {
	char c;
	while (true) {
		c = pgm_read_byte(str);
		if (!c)
			break;
#if BayTCP_DEBUG_INPUT
		BayTCP_DEBUG_INTERFACE.write(c);
#endif

		write(c);
		str++;
	}
}
void BayTCPInterface::printlnPGM(const char *str) {
	printPGM(str);
	println();
#if BayTCP_DEBUG_INPUT
	BayTCP_DEBUG_INTERFACE.println();
#endif
}

void BayTCPInterface::skipChars(void) {
	do {
		while (i_available()) {
#if BayTCP_DEBUG_OUTPUT
			BayTCP_DEBUG_INTERFACE.print((char) read());
#else
			read();
#endif

		}
		delay(1);
	} while (i_available());

}

int BayTCPInterface::strlenURLencoded(const char *str) {
	if (!_urlencode)
		return strlen(str);
	int count = 0;
	while (*str) {
		if (strchr(_urlencodedChars, *str)) {
			count += 3;
		} else
			count++;
		str++;

	}
	return count;
}

void BayTCPInterface::urlDecode(char *str){
	char *leader = str;
	char *follower = str;

	while (*leader) {
	    if (*leader == '%') {
	        leader++;
	        char high = *leader;
	        leader++;
	        char low = *leader;

	        if (high > 0x39) high -= 7;
	        high &= 0x0f;

	        if (low > 0x39) low -= 7;
	        low &= 0x0f;

	        *follower = (high << 4) | low;
	    } else {
	        *follower = *leader;
	    }
	    leader++;
	    follower++;
	}
}


void BayTCPInterface::printURLencoded(const char *str) {
	if (!_urlencode) {
		print(str);
#if BayTCP_DEBUG_INPUT
		BayTCP_DEBUG_INTERFACE.print(str);
#endif
		return;
	}
	while (*str) {
		if (strchr(_urlencodedChars, *str)) {
#if BayTCP_DEBUG_INPUT
			BayTCP_DEBUG_INTERFACE.print('%');
			BayTCP_DEBUG_INTERFACE.print(*str,HEX);
#endif
			print('%');
			print(*str, HEX);
		} else {
#if BayTCP_DEBUG_INPUT
			BayTCP_DEBUG_INTERFACE.print(*str);
#endif
			print(*str);
		}
		str++;
	}
}

void BayTCPInterface::printPostHeader(uint16_t size) {
	printP("POST /");
	print(_path);
#if BayTCP_DEBUG_INPUT
	BayTCP_DEBUG_INTERFACE.print(_path);
#endif
	printlnP(" HTTP/1.1");
	printP("Authorization: Basic ");
	strcpy(_base64buffer+70, _user);
	strcat(_base64buffer+70, ":");
	strcat(_base64buffer+70, _password);
	base64_encode(_base64buffer, (char*) _base64buffer+70, strlen(_base64buffer+70));
	//_base64buffer[base64_enc_len(strlen(_base64buffer+70))] = 0;
	println(_base64buffer); //BASE64
#if BayTCP_DEBUG_INPUT
	BayTCP_DEBUG_INTERFACE.println(_base64buffer);
#endif
	printP("Host: ");
#if BayTCP_DEBUG_INPUT
	BayTCP_DEBUG_INTERFACE.println(_server);
#endif
	println(_server);
	printlnP("User-Agent: BayTCP-1.0");
	printlnP("Content-Type: application/x-www-form-urlencoded");
	printlnP("Connection: close");
	printP("Content-Length: ");
	println(size);
#if BayTCP_DEBUG_INPUT
	BayTCP_DEBUG_INTERFACE.println(size);
#endif
	println();
}

uint8_t BayTCPInterface::sendMultiFromBuffer(uint16_t maxsize) {
	if (!_buffer->available())
		return 0;
	uint8_t res;
	res = connect();
	if (res) {
		_tx_error_count++;
		return (res + 2);
	}

	uint16_t size;
	unsigned long readpos = _buffer->readPos();

	//Guess size for the POST request:
	//We need the size of the POST request for Content-Length. However ATMEGA cannot build up a
	//several kb POST request in RAM. So we send a guess content length and fill up with dummy content
	if (_buffer->available() > ((unsigned long) maxsize)) {
		size = maxsize;
	} else {
		size = _buffer->available();
		size *= 2;
		size += 30;	//Double size and add 30 - This is just a guess. "bayeosframes[]=base64(frame)"
		if (size < 150)
			size = 150; //To small sizes may cause a problem because "bayeosframe[]=" does not fit in...
		size += 7 + strlenURLencoded(_sender); // + 1 + 9 + strlenURLencoded(_password);
		if (size > maxsize)
			size = maxsize;
	}
	printPostHeader(size);
	flushMTU();

	//Send Body - first part (sender)
	printP("sender=");
	printURLencoded(_sender);
//	printP("&password=");
//	printURLencoded(_password);
	uint16_t postsize = 7 + strlenURLencoded(_sender); //+ 1 + 9	+ strlenURLencoded(_password);
	uint16_t mtusize = postsize;

	//Send Body - second part (frames)
	uint8_t framesize;
	while (postsize < (size - 25) && readFromBuffer()) {
		if (_mtu && mtusize > _mtu) {
			flushMTU();
			mtusize = 0;
		}
		base64_encode(_base64buffer, (char*) _payload, getPacketLength());
//		_base64buffer[base64_enc_len(getPacketLength())] = 0;
		framesize = 1 + 15 + strlenURLencoded(_base64buffer);
		postsize += framesize;
		if (postsize <= size) { //Still space in the POST for the frame
			mtusize += framesize;
			printP("&bayeosframes[]=");
			printURLencoded(_base64buffer);
			_buffer->next();
		} else { //Frame does not fitt in the POST
			postsize -= framesize;
			break;
		}
	}

	//Fill up with dummy content
	while (postsize < size) {
		if (_mtu && mtusize > _mtu) {
			flushMTU();
			mtusize = 0;
		}
		print("&");
		postsize++;
		mtusize++;
	}

	println();
	println();
	finishTransmissionMode();
	res = wait_for("HTTP/1.1 200", 30000);
	if (res) {
		_buffer->seekReadPointer(readpos);
		_tx_error_count++;

	} else {
		_tx_error_count = 0;
		delay(10);
	}
	skipChars();

	disconnect();
	return res;
}

uint8_t BayTCPInterface::sendMultiFromBufferWithAckPayload(uint16_t maxsize) {
	if (!_buffer->available())
		return 0;
	uint8_t res;
	res = connect();
	if (res) {
		_tx_error_count++;
		return (res + 2);
	}

	uint16_t size;
	unsigned long readpos = _buffer->readPos();

	//Guess size for the POST request:
	//We need the size of the POST request for Content-Length. However ATMEGA cannot build up a
	//several kb POST request in RAM. So we send a guess content length and fill up with dummy content
	if (_buffer->available() > ((unsigned long) maxsize)) {
		size = maxsize;
	} else {
		size = _buffer->available();
		size *= 2;
		size += 30;	//Double size and add 30 - This is just a guess. "bayeosframes[]=base64(frame)"
		if (size < 150)
			size = 150; //To small sizes may cause a problem because "bayeosframe[]=" does not fit in...
		size += 7 + strlenURLencoded(_sender); // + 1 + 9 + strlenURLencoded(_password);
		if (size > maxsize)
			size = maxsize;
	}
	printPostHeader(size);
	flushMTU();

	//Send Body - first part (sender)
	printP("sender=");
	printURLencoded(_sender);
//	printP("&password=");
//	printURLencoded(_password);
	uint16_t postsize = 7 + strlenURLencoded(_sender); // + 1 + 9 + strlenURLencoded(_password);
	uint16_t mtusize = postsize;

	//Send Body - second part (frames)
	uint8_t framesize;
	while (postsize < (size - 25) && readFromBuffer()) {
		if (_mtu && mtusize > _mtu) {
			flushMTU();
			mtusize = 0;
		}
		base64_encode(_base64buffer, (char*) _payload, getPacketLength());
		//_base64buffer[base64_enc_len(getPacketLength())] = 0;
		framesize = 1 + 15 + strlenURLencoded(_base64buffer);
		postsize += framesize;
		if (postsize <= size) { //Still space in the POST for the frame
			mtusize += framesize;
			printP("&bayeosframes[]=");
			printURLencoded(_base64buffer);
			_buffer->next();
		} else { //Frame does not fitt in the POST
			postsize -= framesize;
			break;
		}
	}

	//Fill up with dummy content
	while (postsize < size) {
		if (_mtu && mtusize > _mtu) {
			flushMTU();
			mtusize = 0;
		}
		print("&");
		postsize++;
		mtusize++;
	}

	println();
	println();
	finishTransmissionMode();
	res = wait_forPGM(PSTR("HTTP/1.1 "),30000,3,_base64buffer);
	if (res || _base64buffer[0]!='2') {
		_buffer->seekReadPointer(readpos);
		_tx_error_count++;
		skipChars();
		disconnect();
		if(_base64buffer[0]!='2') res=1;
		return res;
	}
	_tx_error_count = 0;
	res = wait_forPGM(PSTR("Content-Length: "),5000,2,_base64buffer);

	uint8_t ack_length=atoi(_base64buffer);
	if( ack_length){
		ack_length-=12;
		if(ack_length>BayTCP_BUFFER) ack_length=BayTCP_BUFFER;
		res = wait_forPGM(PSTR("bayeosframe="),5000,ack_length,_base64buffer);
		if(! res){
			urlDecode(_base64buffer);
			_next=base64_decode((char *)_payload, _base64buffer, strlen(_base64buffer));
		}

	} else _next=0;
	skipChars();
	disconnect();
	return 0;

}





uint8_t BayTCPInterface::sendPayload(void) {
#if BayTCP_DEBUG_OUTPUT
	BayTCP_DEBUG_INTERFACE.println("sendPayload");
#endif
	uint8_t res;
	res = connect();
	if (res) {
		_tx_error_count++;
		return (res + 2);
	}

	base64_encode(_base64buffer, (char*) _payload, getPacketLength());
	//_base64buffer[base64_enc_len(getPacketLength())] = 0;

	uint8_t size = strlenURLencoded(_base64buffer);
	size += 7 + strlenURLencoded(_sender) //+ 1 + 9 + strlenURLencoded(_password)
			+ 1 + 15;

	printPostHeader(size);

	//Redo Base64 encoding! buffer is overwritten in printPostHeader!
	base64_encode(_base64buffer, (char*) _payload, getPacketLength());
	//_base64buffer[base64_enc_len(getPacketLength())] = 0;

	printP("sender=");
	printURLencoded(_sender);
//	printP("&password=");
//	printURLencoded(_password);
	printP("&bayeosframes[]=");
	printURLencoded(_base64buffer); //BASE64
	println();
	println(); //was commented out - do we need??
	finishTransmissionMode();
	res = wait_for("HTTP/1.1 200", 30000);
	skipChars();
	if (res)
		_tx_error_count++;
	else
		_tx_error_count = 0;
	disconnect();
	return res;
}

uint8_t BayTCPInterface::addToConfigBuffer(uint8_t offset, const char* str) {
	if (strlen(str) > BayTCP_CONFIG_SIZE - offset - 1) {
		strncpy(_config_buffer + offset, str, BayTCP_CONFIG_SIZE - offset - 1);
		_config_buffer[BayTCP_CONFIG_SIZE - 1] = 0;
		return BayTCP_CONFIG_SIZE - offset;
	} else {
		strcpy(_config_buffer + offset, str);
		return strlen(str) + 1;
	}
}


void BayTCPInterface::readConfigFromStringPGM(const char *string) {
	uint8_t offset = 0;
	while (offset < BayTCP_CONFIG_SIZE) {
		_config_buffer[offset] = pgm_read_byte(string);
		if (!_config_buffer[offset])
			break;
		if (_config_buffer[offset] == '|')
			_config_buffer[offset] = 0;
#if BayTCP_DEBUG_INPUT
		BayTCP_DEBUG_INTERFACE.write(_config_buffer[offset]);
#endif
		offset++;
		string++;
	}
	setConfigPointers();
//	println(_server);
//	println(_port);
}

void BayTCPInterface::readConfigFromEEPROM(int eeoffset) {
	if (EEPROM.read(eeoffset) == 0xff) {
		//No valid config!!
		return;
	}
	for (uint8_t i = 0; i < BayTCP_CONFIG_SIZE; i++) {
		_config_buffer[i] = EEPROM.read(eeoffset + i);
	}
	setConfigPointers();
}

void BayTCPInterface::writeConfigToEEPROM(int eeoffset) {
	for (uint8_t i = 0; i < BayTCP_CONFIG_SIZE; i++) {
		EEPROM.write(eeoffset + i, _config_buffer[i]);
	}
}

const char* BayTCPInterface::getConfig(void) {
	return _config_buffer;
}

char** BayTCPInterface::getConfigPointer(uint8_t index) {
	switch (index) {
	case BayTCP_CONFIG_SERVER:
		return &_server;
		break;
	case BayTCP_CONFIG_PORT:
		return &_port;
		break;
	case BayTCP_CONFIG_PATH:
		return &_path;
		break;
	case BayTCP_CONFIG_USER:
		return &_user;
		break;
	case BayTCP_CONFIG_PASSWORD:
		return &_password;
		break;
	case BayTCP_CONFIG_SENDER:
		return &_sender;
		break;
	case BayTCP_CONFIG_APN:
		return &_apn;
		break;
	case BayTCP_CONFIG_PROVPW:
		return &_prov_pw;
		break;
	case BayTCP_CONFIG_PROVUSER:
		return &_prov_user;
		break;
	case BayTCP_CONFIG_PIN:
		return &_pin;
		break;
	}
	return 0;

}

void BayTCPInterface::setConfigPointers(void) {
	uint8_t offset = 0;
	_server = _config_buffer;
	char** p;
	for (uint8_t i = 1; i < 10; i++) {
		if (_config_buffer[offset])
			offset += strlen(_config_buffer + offset) + 1;
		else
			offset++;
		p = getConfigPointer(i);
		*p = _config_buffer + offset;
	}
}

void BayTCPInterface::setConfig(const char *str, uint8_t index) {
	memcpy(_base64buffer, _config_buffer, BayTCP_CONFIG_SIZE);
	uint8_t offset = 0;
	uint8_t offset_old = 0;

	for (uint8_t i = 0; i < 8; i++) {
		if (i == index)
			offset += addToConfigBuffer(offset, str);
		else
			offset += addToConfigBuffer(offset, _base64buffer + offset_old);
		offset_old += strlen(_base64buffer + offset_old) + 1;
	}
	setConfigPointers();
}

uint8_t BayTCPInterface::wait_for_available(uint16_t* timeout, int bytes) {
	while (i_available() < bytes) {
		delay(1);
		(*timeout)--;
		if (*timeout == 0) {
			return 2;
		}
	}
	return 0;

}

uint8_t BayTCPInterface::wait_forOK(uint16_t timeout) {
	return wait_forPGM(PSTR("OK"), timeout);
}

uint8_t BayTCPInterface::wait_forPGM(const char* str, uint16_t timeout,
		uint8_t bytes, char* buffer) {
	uint8_t length = 0;
	while (true) {
		_base64buffer[length] = pgm_read_byte(str);
		if (!_base64buffer[length])
			break;
		str++;
		length++;
	}

	if (wait_for_available(&timeout, length))
		return 2;
	uint8_t offset = 0;
	char c;
	while (true) {
		if (wait_for_available(&timeout))
			return 2;
		c = read();
#if BayTCP_DEBUG_OUTPUT
		BayTCP_DEBUG_INTERFACE.print(c);
#endif

		if (offset < length) {
			if (c == _base64buffer[offset])
				offset++;
			else
				offset = 0;
			/*		  else if(offset>0){
			 skipChars();
			 return 1;
			 }*/
		}

		if (offset == length) {
			if (bytes && buffer != NULL) {
				if (wait_for_available(&timeout, bytes))
					return 2;
				offset = 0;
				while (offset < bytes) {
					buffer[offset] = read();
					offset++;
				}
				buffer[offset] = 0;
			}
			//skipChars();
			return 0;
		}

	}
}

