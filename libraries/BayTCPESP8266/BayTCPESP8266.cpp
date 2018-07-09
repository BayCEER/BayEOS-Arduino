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

#define ESP8266_DEBUG 0

#if ESP8266_DEBUG
#include <SoftwareSerial.h>
const uint8_t rxPin = 2;
const uint8_t txPin = 3;

// set up a new serial object
SoftwareSerial mySerial(rxPin, txPin);
#define ESP8266_DEBUG_INTERFACE mySerial
#endif

void BayESP8266Interface::powerDown() {
	printlnP("AT+SLEEP=2");
	wait_forOK(200);
	printlnP("AT+GSLP=0");
	wait_forOK(200);
	if (_ch_pdPin > -1) {
		pinMode(_ch_pdPin, OUTPUT);
		digitalWrite(_ch_pdPin, LOW);
	}
}

void BayESP8266Interface::powerUp() {
	if (_ch_pdPin > -1) {
		pinMode(_ch_pdPin, OUTPUT);
		digitalWrite(_ch_pdPin, HIGH);
	}
}

uint8_t BayESP8266Interface::status(){
	printlnP("AT+CIPSTATUS");
	if (!wait_forPGM(PSTR("STATUS:"), 3000, 2, _base64buffer)) {
		return atoi(_base64buffer);
	}
	return 0;
}

uint8_t BayESP8266Interface::connectToAP(){
	printP("AT+CWJAP=\"");
	print(_apn);
	printP("\",\"");
	print(_prov_pw);
	printlnP("\"");
	if (wait_forOK(30000)) return 1;
    return 0;
}

uint8_t BayESP8266Interface::init() {
	i_begin(_baud);
	uint8_t res;
	if (_ch_pdPin > -1) {
		pinMode(_ch_pdPin, OUTPUT);
		digitalWrite(_ch_pdPin, LOW);
		delay(100);
		digitalWrite(_ch_pdPin, HIGH);
	} else {
		printlnP("AT+RST");
	}
	wait_forPGM(PSTR("ready"), 3000);


	res=status();
	if(! res) return 1;//no communication
	if (res == 5) {
		res=connectToAP();
		if(res) return 3;
	}
	printlnP("AT+CIPSSLSIZE=4096");
	wait_forOK(200);
	printlnP("AT+CWMODE=1");
	wait_forOK(400);
	printlnP("AT+CIPMUX=0");
	wait_forOK(400);

	return 0;

}

void BayESP8266Interface::flushMTU(void) {
//	printP("\\0");
//	write((uint8_t) 0);
//	printlnP("AT+CIPSENDEX=2048");
//	wait_for(">", 200);
	flush();
	delay(25);
}

void BayESP8266Interface::finishTransmissionMode(void) {
//	delay(50);
//	printP("\\0");
	flush();
	delay(25);
	printP("+++");
//	write((uint8_t) 0);
}

uint8_t BayESP8266Interface::connect(void) {
	if (_ch_pdPin > -1 && ! digitalRead(_ch_pdPin)) {
		powerUp();
		wait_forPGM(PSTR("ready"), 3000);
	}

	uint8_t res;
	res=status(); //No communication - run init
	if(! res){
		if (init())
			return 1;
	}
	if(res==5){
		if(connectToAP()) return 3;
		else res=status();
	}

	printlnP("AT+CIPMUX=0");
	wait_forOK(2000);

	if(res!=3) {
		printP("AT+CIPSTART=\"");
		if (atoi(_port) == 443)
			printP("SSL");
		else
			printP("TCP");
		printP("\",\"");
		print(_server);
		printP("\",");
		println(_port);
		if (wait_for("CONNECT", 5000))
			return 4;
		wait_forOK(200);
		printlnP("AT+CIPMODE=1");
		wait_forOK(200);
	}

//	printlnP("AT+CIFSR");
//	if (wait_for("ERROR", 200)) {

//		printlnP("AT+CIPSENDEX=2048");
	printlnP("AT+CIPSEND");
	if (wait_for(">", 200))
		return 5;

	return 0;


}

void BayESP8266Interface::disconnect(void) {
//	wait_for("Unlink",1000);
//	printlnP("AT+CIPCLOSE=5");
//	wait_forOK(100);

}

uint8_t BayESP8266Interface::sendATE0(void) {
	skipChars();
	printlnP("ATE0");
	if (wait_forOK(3000))
		return init();
	return 0;
}

BayESP8266::BayESP8266(HardwareSerial &serial, int8_t ch_pdPin) :
		_serial(serial) {
	_urlencode = 1;
	_ch_pdPin = ch_pdPin;
	_mtu = 1500;
}

uint8_t BayESP8266::begin(long baud) {
	_baud = baud;
	return init();
}

int BayESP8266::read(void) {
#if ESP8266_DEBUG
	int c = _serial.read();
	if (c != -1)
		ESP8266_DEBUG_INTERFACE.write(c);
	return c;
#else
	return _serial.read();
#endif

}

size_t BayESP8266::write(uint8_t b) {
#if ESP8266_DEBUG
	ESP8266_DEBUG_INTERFACE.write(b);
#endif
	return _serial.write(b);
}

void BayESP8266::i_begin(long b) {
#if ESP8266_DEBUG
	ESP8266_DEBUG_INTERFACE.begin(9600);
#endif
	_serial.begin(b);
}

BayESP8266softserial::BayESP8266softserial(uint8_t rxPin, uint8_t txPin,
		int8_t ch_pdPin) :
		SoftwareSerial(rxPin, txPin) {
	_urlencode = 1;
	_ch_pdPin = ch_pdPin;
	_mtu = 1500;
}

uint8_t BayESP8266softserial::begin(long baud) {
	_baud = baud;
	return init();
}
