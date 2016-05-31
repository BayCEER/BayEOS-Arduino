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

uint8_t BayGPRSInterface::init(){
	i_begin(_baud);
	uint8_t count=0;
	init_start:
	printP("AT"); //Will autoconfigure BAUD-Rate - Auto BAUD-Rate did not work with Sleep-Mode!
	delay(100);
	println();
	printP("AT+IPR=");
	println(_baud);
	wait_forOK(200);
	for(uint8_t i=0;i<5;i++){
		printlnP("ATE0"); //Command echo off
		if(! wait_forOK(200)){
			//communication ok!
			printlnP("AT+CLTS=1"); //Enable Local Network Time
			wait_forOK(200);
			printlnP("AT+CSCLK=2"); //Auto-Sleepmode
			//Attention: Setting AT+IPR=BAUD-Rate does not work with Auto-Sleepmode enabled!!
			wait_forOK(200);
			//Check PIN
			printlnP("AT+CPIN?");
			wait_forPGM(PSTR("+CPIN: "),30000,7,_base64buffer);
			if(_base64buffer[5]=='U') return 3; //SIM: PUK
			if(_base64buffer[5]=='I'){ //SIM: PIN
				printP("AT+CPIN=\"");
				print(_pin);
				println("\"");
				if(wait_forOK(30000)) {
				  return 2; //Wrong PIN
				}
			}
			printlnP("AT+CPIN?");
			if(wait_for("READY",20000)) return 6; //No SIM

			// Waiting for Modem to Connect
			for(i=0;i<127;i++){
				printlnP("AT+CREG?");
				if(! wait_for(",1",2000)) break;
				delay(200);
			}
			if(i==127) return 4;
			for(i=0;i<127;i++){
				printlnP("AT+CGATT?");
				if(! wait_for("+CGATT: 1",2000)) break;
				delay(200);
			}
			if(i==127) return 5;
			return 0;

		}
	}
	softSwitch();
	count++;
#if BayTCP_DEBUG_OUTPUT
	Serial.print("i-");
#endif

	if(count>1) return 1;
	goto init_start;
}

void BayGPRSInterface::softSwitch(void){
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


uint8_t BayGPRSInterface::setClock(const char* time){
	sendATE0();
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
	sendATE0();
	printlnP("AT+CSQ");
	wait_forPGM(PSTR("+CSQ: "),500,2,_base64buffer);
	return (uint8_t)114 -(2*(uint8_t)atoi(_base64buffer));
}

DateTime BayGPRSInterface::now(void){
	sendATE0();
	printlnP("AT+CCLK?");
	wait_forPGM(PSTR("+CCLK: \""),3000,20,_base64buffer); //YY/MM/DD,HH:MM:SS+02
//	Serial.print("LocalTime:");
//	Serial.println(_base64buffer);
	uint16_t y = atoi(_base64buffer+0) + 2000;
	uint8_t m = atoi(_base64buffer+3);
	uint8_t d = atoi(_base64buffer+6);
	uint8_t hh = atoi(_base64buffer+9);
	uint8_t mm = atoi(_base64buffer+12);
	uint8_t ss = atoi(_base64buffer+15);
	DateTime dt=DateTime (y, m, d, hh, mm, ss);
	dt=DateTime(dt.get()-(3600L*atoi(_base64buffer+17))); //Adjust for Timezone!
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
	if(res=sendATE0()) return res+1;
	setup_start:
	printlnP("AT+CIFSR");
	if(wait_for("ERROR",200)){
		//has IP
		printP("AT+CIPSTART=\"TCP\",\"");
		print(_server);
		printP("\",");
		println(_port);
		if(wait_for("CONNECT",20000)) return 1;

		printlnP("AT+CIPSEND?");
		if(! wait_forPGM(PSTR(": "),200,4,_base64buffer)){
			_base64buffer[4]=0;
			_mtu=atoi(_base64buffer);
			_mtu-=(BayTCP_BUFFER+17);
		} else _mtu=500;


		printlnP("AT+CIPSEND");
		if(wait_for("> ",20000)) return 1;
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
	if(wait_forOK(30000)) goto setup_start;

    goto setup_start;

}

void BayGPRSInterface::disconnect(void){
	//sendATE0();
	printlnP("AT+CIPCLOSE");
	wait_forOK(2000);
//	printlnP("AT+CIPSHUT");
//	wait_forOK(2000);
}

uint8_t BayGPRSInterface::sendATE0(void){
	skipChars();
	printlnP("ATE0");
	if(wait_forOK(3000)) return init();
	return 0;
}

uint8_t BayGPRSInterface::sendSMS(const String &phone, const String &sms){
	sendATE0();
	printlnP("AT+CMGF=1");//humanreadable SMS
	if(wait_forOK(2000)) return 2;
	printP("AT+CMGS=\"");//Target phone number
	print(phone);
	printlnP("\"");
	if(wait_for("> ",2000)) return 2;
	print(sms);
	write(0x1a);
	if(wait_for("+CMGS: ",2000)) return 2;
	return 0;
}


BayGPRS::BayGPRS(HardwareSerial &serial,uint8_t powerPin){
	_serial=&serial;
	_powerPin=powerPin;
	_urlencode=1;
}

uint8_t BayGPRS::begin(long baud){
	_baud=baud;
	return init();
}

BayGPRSsoftserial::BayGPRSsoftserial(uint8_t rxPin, uint8_t txPin,uint8_t powerPin):SoftwareSerial(rxPin,txPin){
	_powerPin=powerPin;
	_urlencode=1;
}

uint8_t BayGPRSsoftserial::begin(long baud){
	_baud=baud;
	return init();
}

