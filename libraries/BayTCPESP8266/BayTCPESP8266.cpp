#include "BayTCPESP8266.h"


uint8_t BayESP8266Interface::init(){
	i_begin(_baud);
	printlnP("AT");
	delay(100);
	if(_resetPin>-1){
		pinMode(_resetPin,OUTPUT);
		digitalWrite(_resetPin,LOW);
		delay(100);
		digitalWrite(_resetPin,HIGH);
	} else {
		printlnP("AT+RST");
	}
	wait_for("ready",3000);

	printP("AT+CIOBAUD=");
	println(_baud);
	wait_forOK(200);

	printlnP("ATE0"); //Command echo off
	if(! wait_forOK(200)){
		//communication ok!
		printlnP("AT+CWMODE=1");
		wait_forOK(400);

		printlnP("AT+CIPMUX=0");
		wait_forOK(400);

		printP("AT+CWJAP=\"");
		print(_apn);
		printP("\",\"");
		print(_prov_pw);
		printlnP("\"");
		if(! wait_forOK(30000)) return 0;
	}

    return 1;
}

void BayESP8266Interface::flushMTU(void){
}

void BayESP8266Interface::finishTransmissionMode(void){
	delay(500);
	printP("+++");
}

uint8_t BayESP8266Interface::connect(void){
	uint8_t res;
	if(res=sendATE0()){
		if(init()) return res;
	}

	printlnP("AT+CIFSR");
	if(wait_for("ERROR",200)){
		//has IP
		printlnP("AT+CIPMUX=0");
		wait_forOK(2000);
		printP("AT+CIPSTART=\"TCP\",\"");
		print(_server);
		printP("\",");
		println(_port);
		wait_for("Linked",200);
		uint8_t i;
		for(i=0;i<10;i++){
			printlnP("AT+CIPSTATUS");
			wait_forPGM(PSTR("STATUS:"),1000,1,_rxBuffer);
			if(_rxBuffer[0]=='3'){
				break;
			}
			delay(500);
		}
		if(i==10) return 1;
		printlnP("AT+CIPMODE=1");
		wait_forOK(200);

		printlnP("AT+CIPSEND");
		if(wait_for(">",200)) return 1;
		return 0;
	}
	return 1;


}

void BayESP8266Interface::disconnect(void){
	wait_for("Unlink",1000);
	printlnP("AT+CIPSTATUS");
	wait_forPGM(PSTR("STATUS:"),1000,1,_rxBuffer);
	if(_rxBuffer[0]!='4'){
		printlnP("AT+CIPCLOSE");
		wait_forOK(100);
	}

}

uint8_t BayESP8266Interface::sendATE0(void){
	skipChars();
	printlnP("ATE0");
	if(wait_forOK(3000)) return init();
	return 0;
}


BayESP8266::BayESP8266(HardwareSerial &serial,int8_t resetPin):HardwareSerial(serial){
	_urlencode=1;
	_resetPin=resetPin;
}

uint8_t BayESP8266::begin(long baud){
	_baud=baud;
	return init();
}

BayESP8266softserial::BayESP8266softserial(uint8_t rxPin, uint8_t txPin,int8_t resetPin):SoftwareSerial(rxPin,txPin){
	_urlencode=1;
	_resetPin=resetPin;
}

uint8_t BayESP8266softserial::begin(long baud){
	_baud=baud;
	return init();
}
