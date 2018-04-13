/*
AT+CIPMUX=0

AT+CIPSTART="TCP","132.180.112.55",80

AT+CIPSENDEX=2048


POST /gateway/frame/saveFlat HTTP/1.1
Authorization: Basic aW1wb3J0OmltcG9ydA==
Host: 132.180.112.55
User-Agent: BayTCP
Content-Type: application/x-www-form-urlencoded
Connection: close
Content-Length: 63

sender=TestWLAN&password=import&bayeosframes[]=AQEAAAAAQA%3D%3D

+++

*/

#include "BayTCPESP8266.h"
void BayESP8266Interface::powerDown(long t){
	printlnP("AT+SLEEP=2");
	wait_forOK(200);
	printP("AT+GSLP=");
	println(t);
	wait_forOK(200);

}



uint8_t BayESP8266Interface::init(){
	i_begin(_baud);
	uint8_t res;
	if(_resetPin>-1){
		pinMode(_resetPin,OUTPUT);
		digitalWrite(_resetPin,LOW);
		delay(100);
		digitalWrite(_resetPin,HIGH);
	} else {
		printlnP("AT+RST");
	}
	wait_for("ready",30000);

	printlnP("ATE0"); //Command echo off
	res=wait_forOK(2000);

	if(! res){
		//communication ok!
		printlnP("AT+CIPSSLSIZE=4096");
		wait_forOK(200);
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
		else return 3;
	}

    return res;
}

void BayESP8266Interface::flushMTU(void){
	printP("\\0");
	printlnP("AT+CIPSENDEX=2048");
	wait_for(">",200);
}

void BayESP8266Interface::finishTransmissionMode(void){
//	delay(50);
	printP("\\0");
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
		printP("AT+CIPSTART=\"");
		if(atoi(_port)==443) printP("SSL");
		else printP("TCP");
		printP("\",\"");
		print(_server);
		printP("\",");
		println(_port);
		if(wait_for("CONNECT",5000)) return 1;
		wait_forOK(200);
		uint8_t i;
		printlnP("AT+CIPMODE=0");
		wait_forOK(200);

		printlnP("AT+CIPSENDEX=2048");
		if(wait_for(">",200)) return 1;
		return 0;
	}
	return 1;


}

void BayESP8266Interface::disconnect(void){
//	wait_for("Unlink",1000);
//		printlnP("AT+CIPCLOSE=5");
//		wait_forOK(100);

}

uint8_t BayESP8266Interface::sendATE0(void){
	skipChars();
	printlnP("ATE0");
	if(wait_forOK(3000)) return init();
	return 0;
}


BayESP8266::BayESP8266(HardwareSerial &serial,int8_t resetPin):_serial(serial){
	_urlencode=1;
	_resetPin=resetPin;
	_mtu=1500;
}

uint8_t BayESP8266::begin(long baud){
	_baud=baud;
	return init();
}

BayESP8266softserial::BayESP8266softserial(uint8_t rxPin, uint8_t txPin,int8_t resetPin):SoftwareSerial(rxPin,txPin){
	_urlencode=1;
	_resetPin=resetPin;
	_mtu=1500;
}

uint8_t BayESP8266softserial::begin(long baud){
	_baud=baud;
	return init();
}
