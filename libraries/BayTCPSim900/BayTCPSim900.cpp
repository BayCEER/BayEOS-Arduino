#include "BayTCPSim900.h"

void RTC_SIM900::adjust(const DateTime& dt) {
	last_millis=millis();
    offset = dt.get();
}

DateTime RTC_SIM900::now() {
	unsigned long diff=(millis()-last_millis)/1000L;
	if(diff>1000000L){
		offset+=diff;
		last_millis=millis();
		diff=0;
	}
    return offset+diff;
}

uint8_t BayGPRSInterface::changeIPR(long baud) {
	_baud = baud;
	long t_baud[] = { baud, 9600, 38400, 57600 };
	for (uint8_t i = 0; i < 4; i++) {
		i_begin(t_baud[i]);
		skipChars();
		uint8_t tries=5;
		do {
			printlnP("AT"); //Will autoconfigure BAUD-Rate - Auto BAUD-Rate did not work with Sleep-Mode!
			if(!wait_forOK(1000)){
				printP("AT+IPR=");
				println(_baud);
				if (!wait_forOK(200)) {
					i_end();
					i_begin(_baud);
					return 0;
				}

			}
			tries--;
		} while(tries);
		i_end();

	}
	return 1;

}
uint8_t BayGPRSInterface::init(uint8_t unlock_only){
	uint8_t count=0;
	init_start:
	uint8_t i=0;
	skipChars();
	printlnP("AT"); //Wake up
	wait_forOK(200);
	printlnP("AT");	
	if(wait_forOK(1000)) return 1;
	//communication ok!
	printlnP("ATE0"); //Command echo off
	wait_forOK(500);
	printlnP("AT+CSCLK=0"); //Auto-Sleepmode
	wait_forOK(500);
	//Check PIN
	printlnP("AT+CPIN?");
	while(wait_forPGM(PSTR("+CPIN: "),5000,7,_base64buffer)){
		printlnP("AT");
		wait_forOK(200);
		printlnP("AT+CFUN=0");
		//Disable
		wait_forOK(10000);
		printlnP("AT+CFUN=1");
		//delay(2000);
		//Enable
		wait_forOK(10000);
		printlnP("AT");
		wait_forOK(200);
		printlnP("AT+CPIN?");
		i++;
		if(i>2) return 6;
	}
	if(_base64buffer[5]=='U') return 3; //SIM PUK
	if(_base64buffer[5]=='I'){ //SIM PIN
		printlnP("AT");
		wait_forOK(200);
		printP("AT+CPIN=\"");
		print(_pin);
		println("\"");
		if(wait_forOK(30000)) {
			return 2; //Wrong PIN
		}
		wait_for("SMS Ready",(unlock_only?5000:60000));
	}
	//Return here - Moden will try to connect and attach in the background!
	if(unlock_only) return 0;
	// Waiting for Modem to Connect
	for(i=0;i<127;i++){
		if(isRegistered()) break;
		delay(500);
	}
	if(i==127) return 4;
	for(i=0;i<127;i++){
		if(isAttached()) break;
		delay(500);
	}
	if(i==127) return 5;
	return 0;
}

void BayGPRSInterface::softSwitch(void){
	if(!_powerPin) return;
#if SIM900_DEBUG
	Serial.println("softSwitch");
#endif
	pinMode(_powerPin, OUTPUT);
	digitalWrite(_powerPin,LOW);
	delay(1000);
	digitalWrite(_powerPin,HIGH);
	delay(2500);
	digitalWrite(_powerPin,LOW);
	delay(3500);
	_tx_error_count=0;
}

void BayGPRSInterface::softReset(void){
	if(!_resetPin) return;
#if SIM900_DEBUG
	Serial.println("softReset");
#endif
	pinMode(_resetPin, OUTPUT);
	digitalWrite(_resetPin,LOW);
	delay(100);
	digitalWrite(_resetPin,HIGH);
	delay(100);
	digitalWrite(_resetPin,LOW);
	delay(100);
	pinMode(_resetPin, INPUT);
}


uint8_t BayGPRSInterface::setClock(const char* time){
	init();
	printP("AT+CCLK=\"");
	print(time);
	printlnP("\"");
//	SerialprintP("setting Clock to:");
//	Serial.print(time);
//	if(wait_forOK(1000)) SerialprintlnP("failed");
//	else SerialprintlnP("ok");
//	return 1;
	return wait_forOK(1000);
}



uint8_t BayGPRSInterface::getRSSI(void){
	printlnP("AT");
	wait_forOK(200);
	printlnP("AT+CSQ");
	wait_forPGM(PSTR("+CSQ: "),500,2,_base64buffer);
	return (uint8_t)114 -(2*(uint8_t)atoi(_base64buffer));
}

uint8_t BayGPRSInterface::isRegistered(void){
	printlnP("AT");
	wait_forOK(200);
	printlnP("AT+CREG?");
	wait_forPGM(PSTR("+CREG: "),2000,3,_base64buffer);
	if(_base64buffer[2]=='1'|| _base64buffer[2]=='5') return 1; //Connected or Roaming
	else return 0;
}

uint8_t BayGPRSInterface::isAttached(void){
	printlnP("AT");
	wait_forOK(200);
	printlnP("AT+CGATT?");
	if(! wait_for("+CGATT: 1",2000)) return 1;
	return 0;
}

DateTime BayGPRSInterface::now(void){
	printlnP("AT");
	wait_forOK(200);
	uint8_t m,d,hh,mm,ss;
	uint16_t y;
	DateTime dt;
	printlnP("AT+CCLK?");
	if(! wait_forPGM(PSTR("+CCLK: \""),3000,20,_base64buffer)){ //YY/MM/DD,HH:MM:SS+02
//	Serial.print("LocalTime:");
//	Serial.println(_base64buffer);
		y = atoi(_base64buffer+0) + 2000;
		m = atoi(_base64buffer+3);
		d = atoi(_base64buffer+6);
		hh = atoi(_base64buffer+9);
		mm = atoi(_base64buffer+12);
		ss = atoi(_base64buffer+15);
		dt=DateTime (y, m, d, hh, mm, ss);
		dt=DateTime(dt.get()-(3600L*atoi(_base64buffer+17))); //Adjust for Timezone!
	}
	return dt;
}
/*
void BayTCPInterface::serialprintPGM(const char *str){
	char c;
	while (true) {
		c=pgm_read_byte(str);
		if (!c) break;
	    Serial.write(c);
	    str++;
	}
}

void BayTCPInterface::serialprintlnPGM(const char *str){
	serialprintPGM(str);
	Serial.println();
}
*/
void BayGPRSInterface::flushMTU(void){
    write((uint8_t) 0x1a);
    wait_forOK(2000);
	printlnP("AT+CIPSEND");
	wait_for("> ",20000);
}

void BayGPRSInterface::finishTransmissionMode(void){
    write((uint8_t) 0x1a);
}


uint8_t BayGPRSInterface::connect(void){
	if(_tx_error_count>20) softSwitch();
	uint8_t count=0;
	uint8_t res;
	if(res=init()) return res+1;
	setup_start:
	printlnP("AT+CIFSR");
	if(wait_for("ERROR",2000)){
		//has IP
		printP("AT+CIPSTART=\"TCP\",\"");
		print(_server);
		printP("\",");
		println(_port);
		if(wait_for("CONNECT",10000)) return 1;

		printlnP("AT+CIPSEND?");
		if(! wait_forPGM(PSTR(": "),200,4,_base64buffer)){
			_base64buffer[4]=0;
			_mtu=atoi(_base64buffer);
			_mtu-=(BayTCP_BUFFER+17);
		} else _mtu=500;


		printlnP("AT+CIPSEND");
		if(wait_for("> ",10000)) return 1;
		return 0;
	}

	count++;
	if(count>100)
		return 1;

	printlnP("AT+CIPMUX=0");
	//if(wait_forOK(400)) goto setup_start;

	printlnP("AT+CIPCLOSE");
	wait_forOK(200);
	printlnP("AT+CIPSHUT");
	wait_forOK(200);


	printP("AT+CSTT=\"");
	print(_apn);
	printP("\",\"");
	print(_prov_user);
	printP("\",\"");
	print(_prov_pw);
	printlnP("\"");
	if(wait_forOK(400)) goto setup_start;

	printlnP("AT+CIICR");
	if(wait_forOK(10000)) goto setup_start;

    goto setup_start;

}

void BayGPRSInterface::disconnect(void){
	//init();
	printlnP("AT+CIPCLOSE");
	wait_forOK(2000);
	printlnP("AT+CSCLK=2"); //Auto-Sleepmode
	wait_forOK(500);
//	printlnP("AT+CIPSHUT");
//	wait_forOK(2000);
}


uint8_t BayGPRSInterface::sendSMS(const String &phone, const String &sms){
	init();
	printlnP("AT+CMGF=1");//humanreadable SMS
	if(wait_forOK(2000)) return 2;
	printP("AT+CMGS=\"");//Target phone number
	print(phone);
	printlnP("\"");
	if(wait_for("> ",2000)) return 3;
	print(sms);
	write(0x1a);
	if(wait_for("+CMGS: ",2000)) return 4;
	return 0;
}

uint8_t BayGPRSInterface::begin(long baud,uint8_t unlock_only){
	_baud=baud;
	i_begin(_baud);
	skipChars();
	printlnP("AT"); //Wake up
	wait_forOK(200);
	printlnP("AT"); //Will autoconfigure BAUD-Rate - Auto BAUD-Rate did not work with Sleep-Mode!
	if(wait_forOK(1000)) changeIPR(baud);
	return init(unlock_only);
}


BayGPRS::BayGPRS(HardwareSerial &serial,uint8_t powerPin,uint8_t resetPin){
	_serial=&serial;
	_powerPin=powerPin;
	_resetPin=resetPin;
	_urlencode=1;
}
int BayGPRS::available(void){return _serial->available();}
int BayGPRS::read(void){
#if SIM900_DEBUG
	int c=_serial->read();
	if(c!=-1) Serial.write(c);
	return c;
#else
	return _serial->read();
#endif

}
void BayGPRS::i_begin(long b){ _serial->begin(b);}
void BayGPRS::i_end(void){ _serial->end();}
int BayGPRS::i_available(void){return _serial->available();}
size_t BayGPRS::write(uint8_t b){
#if SIM900_DEBUG
	Serial.write(b);
#endif
	return _serial->write(b);
}
int BayGPRS::peek(void){return _serial->peek();};
void BayGPRS::flush(void){_serial->flush();};



