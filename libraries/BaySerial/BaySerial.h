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

#include <inttypes.h>
#include <BayEOS.h>
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
	uint8_t readIntoPayload(int timeout=5000);

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
	uint8_t readPacket(uint8_t type=API_DATA);
	void sendByte(uint8_t b, bool escape);
	uint8_t readByte(int timeout, bool escape);
	void sendAck(uint8_t b);

	bool _escape;
	bool _read_timeout;
	int  _timeout;
	uint8_t _length;
	uint8_t _checksumTotal;
	uint8_t _api;
	uint8_t _ack;
	uint8_t _pos;
	uint8_t _break;


};

class BaySerial : public BaySerialInterface {
private:
	HardwareSerial* _serial; //Pointer to existing serial object!!
public:
	/**
	 * Constructor
	 */
	BaySerial(HardwareSerial& serial,int timeout=1000);
	//BaySerial(void);

	int available(void);
	int i_available(void);
	void begin(long baud);
	void flush(void);
	void end(void);
	int read(void);
	size_t write(uint8_t c);

};


#endif
