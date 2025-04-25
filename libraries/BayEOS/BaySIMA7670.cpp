#include "BaySIMA7670.h"

uint8_t BaySIMA7670::begin(long baud)
{
	_baud = baud;
	_serial->begin(_baud);
	skipChars();
	return init();
}

uint8_t BaySIMA7670::init(void)
{
	uint8_t i = 0;
	skipChars();
	if (printlnP_OK("ATE0", 1000))
	{
		pinMode(_power_pin, OUTPUT);
		delay(1000);
		pinMode(_power_pin, INPUT);
		delay(1000);
		while (true)
		{
			printlnP("ATE0");
			if (!wait_forOK(1000))
				break;
			i++;
			if (i > 10)
				return 1;
		}
	}

	i = 0;
	// communication ok!

	printlnP_OK("AT+CTZU=1", 1000); // Networktime
	printlnP_OK("AT+CTZR=1", 1000); // Timezone

	printlnP_OK("AT+CSCLK=0", 500); // Auto-Sleepmode off
	// Check PIN
	// printlnP_OK("AT+CLTS=1",200); //enable clock sync!
	printlnP("AT+CPIN?");
	while (wait_forPGM(PSTR("+CPIN: "), 5000, 7, _base64buffer))
	{
		printlnP_OK("AT+CFUN=0", 10000);
		// Disable
		printlnP_OK("AT+CFUN=1", 10000);
		// Enable
		printlnP("AT+CPIN?");
		i++;
		if (i > 2)
			return 8;
	}
	if (_base64buffer[4] == 'I')
		return 7; // NOT INSERTED
	if (_base64buffer[5] == 'U')
		return 3; // SIM PUK
	if (_base64buffer[5] == 'I')
	{ // SIM PIN
		printP("AT+CPIN=\"");
		getConfig(_pin);
		_serial->println("\"");
		if (wait_forOK(30000))
		{
			return 2; // Wrong PIN
		}
	}
	for (i = 0; i < 127; i++)
	{
		printlnP("AT+CPIN?");
		if (!wait_for("+CPIN: READY", 200))
			break;
	}
	if (i == 127)
		return 4; // No +CPIN: READY

	// Waiting for Modem to Connect
	for (i = 0; i < 127; i++)
	{
		if (isRegistered())
			break;
		delay(500);
	}
	if (i == 127)
		return 5;
	for (i = 0; i < 127; i++)
	{
		if (isAttached())
			break;
		delay(500);
	}
	if (i == 127)
		return 6;
	printlnP_OK("AT+HTTPTERM", 200);

	return 0;
}

uint8_t BaySIMA7670::getRSSI(void)
{
	printlnP("AT+CSQ");
	wait_forPGM(PSTR("+CSQ: "), 500, 2, _base64buffer);
	return (uint8_t)114 - (2 * (uint8_t)atoi(_base64buffer));
}

uint8_t BaySIMA7670::isRegistered(void)
{
	printlnP("AT+CREG?");
	wait_forPGM(PSTR("+CREG: "), 2000, 3, _base64buffer);
	if (_base64buffer[2] == '1' || _base64buffer[2] == '5')
		return 1; // Connected or Roaming
	return 0;
}

uint8_t BaySIMA7670::isAttached(void)
{
	printlnP("AT+CGATT?");
	if (!wait_for("+CGATT: 1", 2000))
		return 1;
	return 0;
}

uint8_t BaySIMA7670::postHeader(uint16_t size)
{
	uint8_t res;
	res = init();
	if (res)
	{
		_tx_error_count++;
		return (res + 5);
	}
	printlnP_OK("AT+HTTPINIT", 200);
	char c;
	uint8_t offset = 0;
	bool password = false;
	while (true)
	{
		c = pgm_read_byte(_user + offset);
		if (c == '|')
		{
			if (password)
				break;
			else
			{
				_base64buffer[70 + offset] = ':';
				password = true;
			}
		}
		else
			_base64buffer[70 + offset] = c;
		offset++;
	}
	_base64buffer[70 + offset] = 0;
	base64_encode(_base64buffer, (char *)_base64buffer + 70, strlen(_base64buffer + 70));
	printP("AT+HTTPPARA=\"USERDATA\",\"Authorization: Basic ");
	_serial->print(_base64buffer);
	printlnP_OK("\"", 200);
	printP("AT+HTTPPARA=\"URL\",\"");
	getConfig(_url);
	if (printlnP_OK("\"", 5000))
		return (3);
	printlnP_OK("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"", 200);
	printP("AT+HTTPDATA=");
	_serial->print(size);
	printlnP(",10000");
	if (wait_forPGM(PSTR("DOWNLOAD"), 200))
		return (4);
	return (0);
}

uint8_t BaySIMA7670::post(void)
{
	printlnP("AT+HTTPACTION=1");
	if (wait_forPGM(PSTR("+HTTPACTION: 1,200"), 25000))
	{
		_tx_error_count++;
		return 1;
	}
	else
	{
		_tx_error_count = 0;
		return 0;
	}
}

void BaySIMA7670::readAck(void)
{
	printlnP("AT+HTTPREAD");
	wait_forPGM(PSTR("+HTTPREAD: "), 5000, 2, _base64buffer);
	uint8_t ack_length = atoi(_base64buffer);
	if (ack_length)
	{
		ack_length -= 12;
		if (ack_length > BaySIM800_BUFFER)
			ack_length = BaySIM800_BUFFER;
		if (!wait_forPGM(PSTR("bayeosframe="), 5000, ack_length, _base64buffer))
		{
			urlDecode(_base64buffer);
			_next = base64_decode((char *)_payload, _base64buffer, ack_length);
		}
	}
	else
		_next = 0;
}
uint8_t BaySIMA7670::sendPayload(void){
	return sendPayloadWithAck(false);
}

uint8_t BaySIMA7670::sendPayloadWithAck(bool ack_payload)
{
	base64_encode(_base64buffer, (char *)_payload, getPacketLength());
	uint8_t size = strlenURLencoded(_base64buffer);
	size += 7 + getConfig(_sender, false, true) + 1 + 15;

	uint8_t res = postHeader(size);
	if (res)
		return (res);
	// Redo Base64 encoding! buffer is overwritten in postHeader!
	base64_encode(_base64buffer, (char *)_payload, getPacketLength());
	printP("sender=");
	getConfig(_sender, true, true);
	printP("&bayeosframes[]=");
	printURLencoded(_base64buffer); // BASE64
	printlnP_OK("", 2000);
	res = post();

	if (ack_payload && !res)
		readAck();
	printlnP_OK("AT+HTTPTERM", 200);
	return res;
}

uint8_t BaySIMA7670::sendMultiFromBuffer(uint16_t maxsize, bool ack_payload)
{
	if (!_buffer->available())
		return 0;
	uint16_t size;
	unsigned long readpos = _buffer->readPos();

	// Guess size for the POST request:
	// We need the size of the POST request for Content-Length. However ATMEGA cannot build up a
	// several kb POST request in RAM. So we send a guess content length and fill up with dummy content
	if (_buffer->available() > ((unsigned long)maxsize))
	{
		size = maxsize;
	}
	else
	{
		size = _buffer->available();
		size *= 2;
		size += 30; // Double size and add 30 - This is just a guess. "bayeosframes[]=base64(frame)"
		if (size < 150)
			size = 150; // To small sizes may cause a problem because "bayeosframe[]=" does not fit in...
		size += 7 + getConfig(_sender, false, true);
		if (size > maxsize)
			size = maxsize;
	}

	uint8_t res = postHeader(size);
	if (res)
		return (res);
	// Send Body - first part (sender)
	printP("sender=");
	uint16_t postsize = 7 + getConfig(_sender, true, true);

	// Send Body - second part (frames)
	uint8_t framesize;
	while (postsize < (size - 25) && readFromBuffer())
	{
		base64_encode(_base64buffer, (char *)_payload, getPacketLength());
		framesize = 1 + 15 + strlenURLencoded(_base64buffer);
		postsize += framesize;
		if (postsize <= size)
		{ // Still space in the POST for the frame
			printP("&bayeosframes[]=");
			printURLencoded(_base64buffer);
			_buffer->next();
		}
		else
		{ // Frame does not fitt in the POST
			postsize -= framesize;
			break;
		}
	}

	// Fill up with dummy content
	while (postsize < size)
	{
		_serial->print("&");
		postsize++;
	}
	printlnP_OK("", 2000);
	res = post();
	if (res)
	{
		_buffer->seekReadPointer(readpos);
	}
	else
	{
		delay(10);
	}

	if (ack_payload && !res)
		readAck();
	printlnP_OK("AT+HTTPTERM", 200);

	return res;
}
