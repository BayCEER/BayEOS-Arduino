#ifndef BayTCPSIM900_h
#define BayTCPSIM900_h

#define SIM900_DEBUG 0


#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <BayTCP.h>
#include <RTClib.h>

class RTC_SIM900 : public RTC {
public:
    void begin() {}
    void adjust(const DateTime& dt);
    DateTime now();

protected:
    unsigned long offset;
    unsigned long last_millis;
};


class BayGPRSInterface : public BayTCPInterface {
public:
	/**
	 * Send SMS
	 * Use international phone numbers +49151xxxxx
	 */
	uint8_t sendSMS(const String &phone, const String &sms);

	/**
	 * Connect to the web through apn
	 * 0 == OK
	 * 1 == Network TIMEOUT
	 * 2 == NO Communication
	 * 3 == PIN failed
	 * 4 == PIN locked
	 * 5 == Not CREG
	 * 6 == Not CGATT
	 * 7 == No SIM Card
	 * Omit, if you just want to send SMS
	 */
	uint8_t connect(void);

	/**
	 * Disconnect from the web
	 */
	void disconnect(void);
	/**
	 * Disconnect from the web
	 */
	uint8_t getRSSI(void);

	/**
	 * Set RTC
	 * time has to be in the format "YY/MM/DD,HH:MM:SS-01"
	 */
	uint8_t setClock(const char* time);

	/**
	 * Get time from RTC
	 */
	DateTime now(void);

	/**
	 * Switch on GPRS-Modem
	 * 0 == GPRS-modem is up and responding OK
	 * 1 == Communication-ERROR
	 * 2 == PIN failed
	 * 3 == PIN locked
	 * 4 == Not CREG
	 * 5 == Not CGATT
	 * 6 == No SIM Card
	 */
	virtual uint8_t begin(long baud);

	/**
	 * perform soft switch operation
	 * will switch on/off modem
	 */
	void softSwitch(void);

	/**
	 * perform software reset operation
	 * only works if a resetPin is set
	 */
	void softReset(void);

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
	 * 2 == PIN failed
	 * 3 == PIN locked
	 * 4 == Not CREG
	 * 5 == Not CGATT
	 * 6 == No SIM Card
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
public:
	uint8_t _powerPin;
	uint8_t _resetPin;
};

class BayGPRS : public BayGPRSInterface {
public:
	/**
	 * Constructor
	 */
	BayGPRS(HardwareSerial &serial, uint8_t powerPin=9,uint8_t resetPin=0);
	uint8_t begin(long baud);
private:
	HardwareSerial* _serial; //Pointer to existing serial object!!
	int available(void){return _serial->available();}
	int read(void){
#if SIM900_DEBUG
		int c=_serial->read();
		if(c!=-1) Serial.write(c);
		return c;
#else
		return _serial->read();
#endif

	}
	void i_begin(long b){ _serial->begin(b);}
	int i_available(void){return _serial->available();}
	size_t write(uint8_t b){
#if SIM900_DEBUG
		Serial.write(b);
#endif
		return _serial->write(b);
	}
    int peek(void){return _serial->peek();};
    void flush(void){_serial->flush();};
};

class BayGPRSsoftserial : private SoftwareSerial, public BayGPRSInterface {
public:
	/**
	 * Constructor
	 */
	BayGPRSsoftserial(uint8_t rxPin, uint8_t txPin, uint8_t powerPin=6, uint8_t resetPin=0);
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
