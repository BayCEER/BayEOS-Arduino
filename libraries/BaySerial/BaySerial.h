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


class BaySerial : public HardwareSerial, public BayEOS {
public:
	/**
	 * Constructor
	 */
	BaySerial(HardwareSerial &serial=Serial,int timeout=1000);

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

	int available(void){
		return HardwareSerial::available();
	}
	void begin(long baud);
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


#endif
