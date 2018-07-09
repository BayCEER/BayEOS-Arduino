#ifndef BayTCPESP8266_h
#define BayTCPESP8266_h


#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <BayTCP.h>


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
	 * Checks status of Modem
	 * 0 == no response
	 * 5 == not connected to AP
	 * 2 == Got IP
     * 3 == TCP Connected
     * 4 == TCP Disconnected
	 *
	 */
	uint8_t status(void);

	/**
	 * Connect to AP
	 * 0 == OK
	 * 1 == Not connected
	 *
	 */
	uint8_t connectToAP(void);

	/**
	 * Switch on GPRS-Modem
	 * 0 == OK
	 * 1 == NO Communication
	 *
	 */
	uint8_t init(void);


	/**
	 * Power down modem for t ms
	 *
	 * Modem enters deep sleep - wakeup only works with RST
	 * powerDown time is limited to 70 min
	 */
	void powerDown();
	void powerUp();

	virtual void i_begin(long baud)=0;

	/*
	 * Flush the transfer depending on MTU of Network
	 */
	void flushMTU(void);

	/*
	 * Finish the transmission mode of the modem
	 */
    void finishTransmissionMode(void);

	long _baud;
    int8_t _ch_pdPin;
    char _rxBuffer[2];

};

class BayESP8266 : public BayESP8266Interface {
public:
	/**
	 * Constructor
	 */
	BayESP8266(HardwareSerial &serial=Serial,int8_t ch_pdPin=-1);
	uint8_t begin(long baud);
private:
	HardwareSerial& _serial; //Reference to existing serial object!!
	int available(void){return _serial.available();}
	int read(void);
	void i_begin(long b);
	int i_available(void){return _serial.available();}
	size_t write(uint8_t b);
    int peek(void){return _serial.peek();};
    void flush(void){_serial.flush();};
};


class BayESP8266softserial : private SoftwareSerial, public BayESP8266Interface {
public:
	/**
	 * Constructor
	 */
	BayESP8266softserial(uint8_t rxPin, uint8_t txPin, int8_t ch_pdPin);
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
