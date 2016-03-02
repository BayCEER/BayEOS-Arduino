/**
 * Arduino BayXBee Class extends XBee and BayEOS Class
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
 *
 * Class to send data and messages to a BayXBee Router
 *
 */

#ifndef BayXBee_h
#define BayXBee_h

#define ENABLE_RX 1
 
#define XBeeError_Timeout 0x2
#define XBeeError_NoSuccess 0x1

#ifndef BayEOS_MAX_PAYLOAD
#define BayEOS_MAX_PAYLOAD 100
#endif

#ifndef BAYXBEE_GATEWAY
#define BAYXBEE_GATEWAY 0x1
#endif

#include "../XBee/XBee.h"
#include "../BayEOS/BayEOS.h"
#include <inttypes.h>
#include <WString.h>


class BayXBeeInterface : public XBeeInterface, public BayEOS{
public:
	/**
	 * Default constructor
	 * To enable Sleep node configure XBee with:
	 * - ATSM1    => Sleep mode - pin hibernate
	 * - ATD70     => CTS flow control disabled
	 *
	 * Attention: Setting a wait_time_for_response to 0 will automatically disable
	 * waiting for response in sendPayload
	 */
	BayXBeeInterface(uint8_t sleep_pin=0, uint8_t wakeup_time=15, int wait_time_for_response=5000,uint16_t dest=BAYXBEE_GATEWAY);

	/**
	 * Send current payload buffer to dest
	 */
	uint8_t sendPayload(void);

	int available(void){
		return 0; //Note this is a dummy to make BayXBee to compile!
	}

	uint16_t getPANID(void);

	uint8_t parseRX16(BayEOS &client, int rx_panid);


#if ENABLE_RX
	uint8_t readIntoPayload(int timeout=5000);
#endif

	/**
	 * Start BayXBee
	 */
	void begin(long baud);

	virtual void i_begin(long baud) = 0;


private:
	Tx16Request _tx;
	TxStatusResponse _txStatus;
	uint8_t _sleepPin;
	uint8_t _wakeupTime;
	int _waitTimeForResponse;
	uint16_t _destination;
#if ENABLE_RX
	Rx16Response _rx;
#endif
};

class BayXBee: public BayXBeeInterface {
private:
	HardwareSerial* _serial;

public:
	BayXBee(HardwareSerial& serial, uint8_t sleep_pin=0, uint8_t wakeup_time=15, int wait_time_for_response=5000,uint16_t dest=BAYXBEE_GATEWAY):
		BayXBeeInterface(sleep_pin,wakeup_time,wait_time_for_response,dest)
	{
		_serial = &serial;
	}

	void setSerial(HardwareSerial &serial){
		_serial = &serial;
	}

	int i_available(void){
		return _serial->available();
	}
	void i_begin(long baud){
		_serial->begin(baud);
	}
	void flush(void){
		_serial->flush();
	}
	int read(void){
		return _serial->read();
	}
	size_t write(uint8_t c){
		return _serial->write(c);
	}



};

/*
 * Get PANID of XBee
 */
/*uint16_t getPANID(XBee &xbee);
uint8_t parseRX16(BayEOS &client, XBee &xbee,int rx_panid);
*/
#endif


