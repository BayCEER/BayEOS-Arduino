#ifndef BayTCPESP8266_h
#define BayTCPESP8266_h


#include <HardwareSerial.h>
#include <BayTCP.h>


class BayESP8266Interface : public BayTCPInterface {
public:
	/**
	 * Connect to the web through access point
	 * 0 == OK
	 * 1 == No Connect to AP
	 * 2 == Modem TIMEOUT
	 * 3 == No Connect to server
	 * 4 == No send
	 */
	uint8_t connect(void);

	/**
	 * Disconnect from the web
	 */
	void disconnect(void);


	/**
	 * Switch on Modem
	 * 0 == Modem is up and responding OK
	 * 1 == No Connect to AP
	 * 2 == Modem Timeout
	 */
	virtual uint8_t begin(long baud);

	/**
	 * 0 == OK
	 * 1 == ERROR
	 *
	 */
	uint8_t changeIPR(long baud);

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
	 * Power down modem
	 *
	 */
	void powerDown();
	void powerUp();

	virtual void i_begin(long baud)=0;
	virtual void i_end()=0;

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

};

class BayESP8266 : public BayESP8266Interface {
public:
	/**
	 * Constructor
	 */
	BayESP8266(HardwareSerial &serial,int8_t ch_pdPin=-1);
	uint8_t begin(long baud);
private:
	HardwareSerial* _serial; //Pointer serial object!!
	int available(void);
	int read(void);
	void i_begin(long b);
	void i_end();
	int i_available(void);
	size_t write(uint8_t b);
    int peek(void);
    void flush(void);
};



#endif
