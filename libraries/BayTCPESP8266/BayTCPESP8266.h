#ifndef BayTCPESP8266_h
#define BayTCPESP8266_h

#define ESP8266_DEBUG 0


#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include "../BayTCP/BayTCP.h"


class BayESP8266Interface : public BayTCPInterface {
public:
	/**
	 * Connect to the web through access point
	 * 0 == OK
	 * 1 == No Access point
	 * 2 == Modem TIMEOUT
	 */
	uint8_t connect(void);

	/**
	 * Disconnect from the web
	 */
	void disconnect(void);


	/**
	 * Switch on Modem
	 * 0 == Modem is up and responding OK
	 * 1 == Communication-ERROR
	 *
	 */
	virtual uint8_t begin(long baud);

	/**
	 * Send ATE0 command
	 * will init moden if modem does not respond...
	 * 0 == GPRS-modem is up and responding OK
	 * 1 == Error
	 */
	uint8_t sendATE0(void);

	/**
	 * Switch on GPRS-Modem
	 * 0 == OK
	 * 1 == NO Communication
	 *
	 */
	uint8_t init(void);



	virtual void i_begin(long baud);

	/*
	 * Flush the transfer depending on MTU of Network
	 */
	void flushMTU(void);

	/*
	 * Finish the transmission mode of the modem
	 */
    void finishTransmissionMode(void);

	long _baud;
    int8_t _resetPin;
    char _rxBuffer[2];

};

class BayESP8266 : private HardwareSerial, public BayESP8266Interface {
public:
	/**
	 * Constructor
	 */
	BayESP8266(HardwareSerial &serial=Serial,int8_t resetPin=-1);
	uint8_t begin(long baud);
private:
	int available(void){return HardwareSerial::available();}
	int read(void){
#if ESP8266_DEBUG
		int c=HardwareSerial::read();
		if(c!=-1) Serial.write(c);
		return c;
#else
		return HardwareSerial::read();
#endif

	}
	void i_begin(long b){ HardwareSerial::begin(b);}
	int i_available(void){return HardwareSerial::available();}
	size_t write(uint8_t b){
#if ESP8266_DEBUG
		Serial.write(b);
#endif
		return HardwareSerial::write(b);
	}
    int peek(void){return HardwareSerial::peek();};
    void flush(void){HardwareSerial::flush();};
};


class BayESP8266softserial : private SoftwareSerial, public BayESP8266Interface {
public:
	/**
	 * Constructor
	 */
	BayESP8266softserial(uint8_t rxPin, uint8_t txPin, int8_t resetPin);
	uint8_t begin(long baud);
private:
	int available(void){return SoftwareSerial::available();}
	int read(void){return SoftwareSerial::read();}
	void i_begin(long b){ SoftwareSerial::begin(b);}
	int i_available(void){return SoftwareSerial::available();}
	size_t write(uint8_t b){return SoftwareSerial::write(b);}
    int peek(void){return SoftwareSerial::peek();};
    void flush(void){SoftwareSerial::flush();};
};


#endif
