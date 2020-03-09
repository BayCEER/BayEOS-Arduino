/**
 * Arduino BaySerial
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
 *
 * Class for sending BayEOS-Frames over a serial Line
 *
 * Frames send over serial:
 * [0x7e][length_of_data][API-ID][------------DATA-----------][CHECKSUM]
 *
 *
 * START_BYTE,ESCAPE,XON,XOFF in [length,api-id,data,checksum] will get escaped
 * (see below)
 *
 *
 */

#ifndef BaySerial_h
#define BaySerial_h

#define START_BYTE 0x7e
#define ESCAPE 0x7d
#define XON 0x11
#define XOFF 0x13

#define API_DATA 0x1
#define API_ACK 0x2

#define TX_OK 0x1
#define TX_CHECKSUM_FAILED 0x2
#define TX_BREAK 0x3
#define TX_BUSY 0x4

#include <inttypes.h>
#include <BayEOS.h>
#include <BayEOSCommands.h>
#include <HardwareSerial.h>
#include <Arduino.h>

class BaySerialInterface : public BayEOS {
public:
	/**
	 * Send the BayEOS payload
	 * returns 0 for success
	 * 1 == no success
	 * 2 == timeout
	 * 3 == break
	 * 4 == cts_busy
	 */
	uint8_t sendPayload(void);

	/**
	 * Send the BayEOS payload
	 * without waiting for a ACK
	 */
	void sendFrame(void);

	/**
	 * Read a frame from rx
	 * returns 0 for success
	 * 1 == no success (Checksum failure)
	 * 2 == timeout
	 */
	uint8_t readIntoPayload(int timeout=0);

	void sendTXBreak(void){
		sendAck(TX_BREAK);
	}

	virtual int i_available(void)=0;
	virtual void begin(long baud)=0;
 	virtual int read(void)=0;
	virtual size_t write(uint8_t c)=0;
	virtual void flush(void)=0;
	virtual void end(void)=0;

protected:
	uint8_t readPacket(uint8_t type=API_DATA, int timeout=0);
	void sendByte(uint8_t b, bool escape);
	uint8_t readByte(bool escape);
	void sendAck(uint8_t b);

	bool _escape;
	bool _read_timeout;
	unsigned long _start;
	uint16_t _timeout;
	uint16_t _current_timeout;
	uint8_t _length;
	uint8_t _checksumTotal;
	uint8_t _api;
	uint8_t _ack;
	uint8_t _pos;
	uint8_t _break;
	uint8_t _cts_pin=0;
	uint8_t _retries=0;
	long _baud;


};

class BaySerial : public BaySerialInterface {
protected:
	HardwareSerial* _serial; //Pointer to existing serial object!!
public:
	/**
	 * Constructor
	 */
	BaySerial(HardwareSerial& serial,int timeout=1000,long baud=38400,uint8_t cts_pin=0);
	//BaySerial(void);

	int available(void);
	int i_available(void);
	void begin(long baud);
	void flush(void);
	void end(void);
	int read(void);
	size_t write(uint8_t c);

};

/*
 * BaySerialESP uses BaySerial to communicate with a
 * specially programmed ESP-Router-Chip (BayESP8266 Library)
 *
 * The ESP-Chip can be attached to the normal FTDI port of the
 * BayEOS-Board.
 *
 */

class BaySerialESP : public BaySerial{
public:
	/*
	 * Constructor
	 */
	BaySerialESP(HardwareSerial& serial, uint8_t ch_pd_pin=0, int timeout=1000);
	/*
	 * Sends data from buffer to esp and calls send command
	 *
	 * returns:
	 * 0 == OK
	 * other result codes see implementation
	 */
	uint8_t sendMultiFromBuffer(uint16_t maxsize=1000);

	/*
	 * Checks if Router is responding
	 * returns
	 * 0 == OK
	 * 1 == not connected to WIFI
	 * 2 == no ack
	 * 3 == no response
	 */
	uint8_t isReady();


	/*
	 * Power Up ESP via ch_pd-Pin and call isReady()
	 */
	uint8_t powerUp(uint8_t tries=20);

	void powerDown(void);


private:
	uint8_t _ch_pd_pin;
};

#endif
