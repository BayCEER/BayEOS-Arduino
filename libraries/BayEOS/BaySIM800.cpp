#include "BaySIM800.h"
const char* const BaySIM800::_urlencodedChars = "$&+,/:;=?@ <>#%{}|~[]`";


BaySIM800::BaySIM800(HardwareSerial &serial){
   _serial=&serial;
}

uint8_t BaySIM800::begin(long baud){
    _baud=baud;
	_serial->begin(_baud);
	skipChars();
	printlnP_OK("AT",200); //Wake up
	printlnP("AT"); //Will autoconfigure BAUD-Rate - Auto BAUD-Rate did not work with Sleep-Mode!
	if(wait_forOK(1000)) changeIPR(baud);
	return init();
}

uint8_t BaySIM800::init(void){
	uint8_t i=0;
	skipChars();
	printlnP_OK("AT",200); //Wake up
	printlnP("AT");	
	if(wait_forOK(1000)) return 1;
	//communication ok!
	printlnP_OK("ATE0",500); //Command echo off
	printlnP_OK("AT+CSCLK=0",500); //Auto-Sleepmode off
	//Check PIN
	printlnP_OK("AT+CLTS=1",200); //enable clock sync!
	printlnP("AT+CPIN?");
	while(wait_forPGM(PSTR("+CPIN: "),5000,7,_base64buffer)){
		printlnP_OK("AT",200);
		printlnP_OK("AT+CFUN=0",10000);
		//Disable
		printlnP_OK("AT+CFUN=1",10000);
		//Enable
		printlnP_OK("AT",200);
		printlnP("AT+CPIN?");
		i++;
		if(i>2) return 8;
	}
	if(_base64buffer[4]=='I') return 7; //NOT INSERTED
	if(_base64buffer[5]=='U') return 3; //SIM PUK
	if(_base64buffer[5]=='I'){ //SIM PIN
		printlnP_OK("AT",200);
		printP("AT+CPIN=\"");
		getConfig(_pin);
		_serial->println("\"");
		if(wait_forOK(30000)) {
			return 2; //Wrong PIN
		}
	}
	for(i=0;i<127;i++){
		printlnP("AT+CPIN?");
		if(! wait_for("+CPIN: READY",200)) break;
	}
	if(i==127) return 4; //No +CPIN: READY

	// Waiting for Modem to Connect
	for(i=0;i<127;i++){
		if(isRegistered()) break;
		delay(500);
	}
	if(i==127) return 5;
	for(i=0;i<127;i++){
		if(isAttached()) break;
		delay(500);
	}
	if(i==127) return 6;
    printlnP_OK("AT+HTTPTERM",200);
    printlnP_OK("AT+SAPBR=0,1",200);
    printlnP_OK("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",200);
    printP("AT+SAPBR=3,1,\"APN\",\"");
    getConfig(_apn);
	printlnP_OK("\"",200);
    wait_forOK(200);
    printP("AT+SAPBR=3,1,\"USER\",\"");
    getConfig(_prov_user);
	printlnP_OK("\"",200);
    printP("AT+SAPBR=3,1,\"PWD\",\"");
    getConfig(_prov_pw);
	printlnP_OK("\"",200);
    
	return 0;
}


uint8_t BaySIM800::changeIPR(long baud) {
	_baud = baud;
	uint16_t t_baud[] = { baud, 9600, 38400, 57600 };
	for (uint8_t i = 0; i < 4; i++) {
		_serial->begin(t_baud[i]);
		skipChars();
		uint8_t tries=5;
		do {
			printlnP("AT"); //Will autoconfigure BAUD-Rate - Auto BAUD-Rate did not work with Sleep-Mode!
			if(!wait_forOK(1000)){
				printP("AT+IPR=");
				_serial->println(_baud);
				if (!wait_forOK(200)) {
					_serial->end();
					_serial->begin(_baud);
					return 0;
				}

			}
			tries--;
		} while(tries);
		_serial->end();

	}
	return 1;

}


uint8_t BaySIM800::getRSSI(void){
	printlnP_OK("AT",200);
	printlnP("AT+CSQ");
	wait_forPGM(PSTR("+CSQ: "),500,2,_base64buffer);
	return (uint8_t)114 -(2*(uint8_t)atoi(_base64buffer));
}

uint8_t BaySIM800::isRegistered(void){
	printlnP_OK("AT",200);
	printlnP("AT+CREG?");
	wait_forPGM(PSTR("+CREG: "),2000,3,_base64buffer);
	if(_base64buffer[2]=='1'|| _base64buffer[2]=='5') return 1; //Connected or Roaming
	return 0;
}

uint8_t BaySIM800::isAttached(void){
	printlnP_OK("AT",200);
	printlnP("AT+CGATT?");
	if(! wait_for("+CGATT: 1",2000)) return 1;
	return 0;
}

DateTime BaySIM800::now(void){
	printlnP_OK("AT",200);
	uint8_t m,d,hh,mm,ss;
	uint16_t y;
	DateTime dt;
	printlnP("AT+CCLK?");
	if(! wait_forPGM(PSTR("+CCLK: \""),3000,20,_base64buffer)){ //YY/MM/DD,HH:MM:SS-08 
//"-08" expressed in quarters of an hour, between the local time and GMT; 
//	Serial.print("LocalTime:");
//	Serial.println(_base64buffer);
		y = atoi(_base64buffer+0) + 2000;
		m = atoi(_base64buffer+3);
		d = atoi(_base64buffer+6);
		hh = atoi(_base64buffer+9);
		mm = atoi(_base64buffer+12);
		ss = atoi(_base64buffer+15);
		dt=DateTime (y, m, d, hh, mm, ss);
		dt=DateTime(dt.get()-(900L*atoi(_base64buffer+17))); //Adjust for Timezone!
	}
	return dt;
}


uint8_t BaySIM800::postHeader(uint16_t size){
	uint8_t res;
	res = init();
	if (res) {
		_tx_error_count++;
		return (res + 5);
	}
    printlnP_OK("AT+SAPBR=1,1",200);
	uint8_t i=0;
	for(i=0;i<10;i++){
		delay(300);
		printlnP("AT+SAPBR=2,1");
		if(! wait_forPGM(PSTR("1,1,"),1000)) break;
	}
	if(i>=9) return(2);
    printlnP_OK("AT+HTTPINIT",200);
    printlnP_OK("AT+HTTPPARA=\"CID\",1",200);
    printlnP_OK("AT+HTTPPARA=\"UA\",\"BaySIM800 1.1\"",200);
	char c;
	uint8_t offset=0;
	bool password=false;
	while(true){
		c=pgm_read_byte(_user+offset);
		if(c=='|'){
		  if(password) break;
		  else{
			_base64buffer[70+offset]=':';
			password=true;
		  }
		} else 
			_base64buffer[70+offset]=c;
		offset++;
	}
	_base64buffer[70+offset]=0;
	base64_encode(_base64buffer, (char*) _base64buffer+70, strlen(_base64buffer+70));
	printP("AT+HTTPPARA=\"USERDATA\",\"Authorization: Basic ");
	_serial->print(_base64buffer);
	printlnP_OK("\"",200);
    printP("AT+HTTPPARA=\"URL\",\"");
    getConfig(_url);
    if(printlnP_OK("\"",5000)) return(3);
    printlnP_OK("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"",200);
    printP("AT+HTTPDATA=");
    _serial->print(size);
    printlnP(",10000");
    if(wait_forPGM(PSTR("DOWNLOAD"),200)) return(4);
	return(0);
}

uint8_t BaySIM800::post(void){
	if(pgm_read_byte(_url+4)=='s'){ //https
		printlnP_OK("AT+HTTPSSL=1",200);
		printlnP_OK("AT+SSLOPT=0,1",200);	
	}
    printlnP("AT+HTTPACTION=1");
    if(wait_forPGM(PSTR("+HTTPACTION: 1,200"),25000)){
		_tx_error_count++;
		return 1;
	} else {
		_tx_error_count=0;
		return 0;
	}
}

void BaySIM800::readAck(void){
    printlnP("AT+HTTPREAD");
    wait_forPGM(PSTR("+HTTPREAD: "),5000,2,_base64buffer);
	uint8_t ack_length=atoi(_base64buffer);
	if( ack_length){
		ack_length-=12;
		if(ack_length>BaySIM800_BUFFER) ack_length=BaySIM800_BUFFER;
		if(! wait_forPGM(PSTR("bayeosframe="),5000,ack_length,_base64buffer)){
			urlDecode(_base64buffer);
			_next=base64_decode((char *)_payload, _base64buffer, ack_length);
		}
	} else _next=0;
}

uint8_t BaySIM800::sendPayload(void){
	return sendPayloadWithAck(false);
}

uint8_t BaySIM800::sendPayloadWithAck(bool ack_payload){
	base64_encode(_base64buffer, (char*) _payload, getPacketLength());
	uint8_t size = strlenURLencoded(_base64buffer);
	size += 7 + getConfig(_sender,false,true) + 1 + 15;

	uint8_t res=postHeader(size);
	if(res) return(res);
	//Redo Base64 encoding! buffer is overwritten in postHeader!
	base64_encode(_base64buffer, (char*) _payload, getPacketLength());
	printP("sender=");
	getConfig(_sender,true,true);
	printP("&bayeosframes[]=");
	printURLencoded(_base64buffer); //BASE64
	printlnP_OK("",2000);	
	res=post();

    if(ack_payload && ! res) readAck();
    printlnP_OK("AT+HTTPTERM",200);
    printlnP_OK("AT+SAPBR=0,1",200);
	printlnP_OK("AT+CSCLK=2",500); //Auto-Sleepmode
	return res;	

}

uint8_t BaySIM800::sendMultiFromBuffer(uint16_t maxsize,bool ack_payload) {
	if (!_buffer->available())
		return 0;
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
		size += 7 + getConfig(_sender,false,true); 
		if (size > maxsize)
			size = maxsize;
	}

	uint8_t res=postHeader(size);
	if(res) return(res);
	//Send Body - first part (sender)
	printP("sender=");
	uint16_t postsize = 7 + getConfig(_sender,true,true); 

	//Send Body - second part (frames)
	uint8_t framesize;
	while (postsize < (size - 25) && readFromBuffer()) {
		base64_encode(_base64buffer, (char*) _payload, getPacketLength());
		framesize = 1 + 15 + strlenURLencoded(_base64buffer);
		postsize += framesize;
		if (postsize <= size) { //Still space in the POST for the frame
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
		_serial->print("&");
		postsize++;
	}
	printlnP_OK("",2000);	
	res=post();
	if (res) {
		_buffer->seekReadPointer(readpos);
	} else {
		delay(10);
	}

    if(ack_payload && ! res) readAck();
    printlnP_OK("AT+HTTPTERM",200);
    printlnP_OK("AT+SAPBR=0,1",200);
	printlnP_OK("AT+CSCLK=2",500); //Auto-Sleepmode

	return res;
}



void BaySIM800::printPGM(const char *str) {
	skipChars();
	char c=pgm_read_byte(str);;
	while (c) {
		_serial->write(c);
		str++;
		c = pgm_read_byte(str);
	}
}

void BaySIM800::printlnPGM(const char *str) {
    printPGM(str);
    _serial->println();
}


uint8_t BaySIM800::printlnPGM(const char *str,uint16_t timeout) {
	printlnPGM(str);
    return wait_forOK(timeout);
}

void BaySIM800::skipChars(void) {
	do {
		while (_serial->available()) {
			_serial->read();

		}
		delay(1);
	} while (_serial->available());

}

void BaySIM800::urlDecode(char *str){
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

uint8_t BaySIM800::URLencoded(char c, bool print)
{
	if (strchr(_urlencodedChars, c))
	{
		if (print)
		{
			_serial->print('%');
			_serial->print(c, HEX);
		}
		return 3;
	}
	else
	{
		if (print)
			_serial->print(c);
		return 1;
	}
}

uint8_t BaySIM800::strlenURLencoded(const char *str) {
	uint8_t count = 0;
	while (*str) {
		if (strchr(_urlencodedChars, *str)) {
			count += 3;
		} else
			count++;
		str++;
	}
	return count;
}



void BaySIM800::printURLencoded(const char *str) {
	while (*str) {
		if (strchr(_urlencodedChars, *str)) {
			_serial->print('%');
			_serial->print(*str, HEX);
		} else {
			_serial->print(*str);
		}
		str++;
	}
}


uint8_t BaySIM800::getConfig(const char * config,bool print,bool urlencoded){
	uint8_t count = 0;
	char c;
	while(true){
	  c=pgm_read_byte(config);
	  if(c=='|') break;
	  if(! c) break;
	  if(print){
		if(urlencoded) count+=URLencoded(c,true);
		else{
			count++;
			_serial->print(c);
		}
	  } else {
		if(urlencoded) count+=URLencoded(c,false);
		else count++;		
	  }
	  config++;
	}
	return count;
}


void BaySIM800::readConfigFromStringPGM(const char *string) {
	_url=string;
	_user=_url+getConfig(_url,false)+1;
	_password=_user+getConfig(_user,false)+1;
	_sender=_password+getConfig(_password,false)+1;
	_apn=_sender+getConfig(_sender,false)+1;
	_prov_user=_apn+getConfig(_apn,false)+1;
	_prov_pw=_prov_user+getConfig(_prov_user,false)+1;
	_pin=_prov_pw+getConfig(_prov_pw,false)+1;
}


uint8_t BaySIM800::wait_for_available(int bytes) {
	while (_serial->available() < bytes) {
		delay(1);
		if (_timeout == 0) {
			return 1;
		}
		_timeout--;
	}
	return 0;
}

uint8_t BaySIM800::wait_forOK(uint16_t timeout) {
	return wait_forPGM(PSTR("OK"), timeout);
}

uint8_t BaySIM800::wait_forPGM(const char* str, uint16_t timeout,
		uint8_t bytes, char* buffer) {
	_timeout=timeout;
	uint8_t length = 0;
	while (true) {
		_base64buffer[length] = pgm_read_byte(str);
		if (!_base64buffer[length])
			break;
		str++;
		length++;
	}

	if (wait_for_available(length))
		return 1;
	uint8_t offset = 0;
	char c;
	while (true) {
		if (wait_for_available())
			return 1;
		c = _serial->read();
		if (offset < length) {
			if (c == _base64buffer[offset])
				offset++;
			else
				offset = 0;
		}

		if (offset == length) {
			if (bytes && buffer != NULL) {
				if (wait_for_available(bytes))
					return 1;
				offset = 0;
				while (offset < bytes) {
					buffer[offset] = _serial->read();
					offset++;
				}
				buffer[offset] = 0;
			}
			return 0;
		}

	}
}


