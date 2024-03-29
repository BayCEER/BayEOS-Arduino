#ifndef BayTCPSIM900_h
#define BayTCPSIM900_h

#define SIM900_DEBUG 0


#include <HardwareSerial.h>
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
	 *
	 * if unlock_only is set, function returns already after unlocking the modem
	 */
	uint8_t begin(long baud,uint8_t unlock_only=0);

	/*
	 * Change the baud rate of the Modem
	 * 0 == OK
	 * 1 == Failed
	 */
	uint8_t changeIPR(long baud);

	/**
	 * Checks if modem is registered to network
	 */
	uint8_t isRegistered(void);

	/**
	 * Checks if modem is attached to network
	 */
	uint8_t isAttached(void);

	/**
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
	uint8_t init(uint8_t unlock_only=0);



	virtual void i_begin(long baud)=0;
	virtual void i_end(void)=0;

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
private:
	HardwareSerial* _serial; //Pointer to existing serial object!!
	int available(void);
	int read(void);
	void i_begin(long b);
	void i_end(void);
	int i_available(void);
	size_t write(uint8_t b);
    int peek(void);
    void flush(void);
};




#endif
