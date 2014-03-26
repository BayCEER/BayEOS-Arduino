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


class BayXBee : public XBee, public BayEOS{
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
	BayXBee(uint8_t sleep_pin=0, uint8_t wakeup_time=15, int wait_time_for_response=5000,uint16_t dest=BAYXBEE_GATEWAY);

	/**
	 * Send current payload buffer to dest
	 */
	uint8_t sendPayload(void);

	int available(void){
		return 1; //Note this is a dummy to make BayXBee to compile!
	}

#if ENABLE_RX
	uint8_t readIntoPayload(int timeout=5000);
#endif

	/**
	 * Start BayXBee
	 */
	void begin(long baud);


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

/*
 * Get PANID of XBee
 */
uint16_t getPANID(XBee &xbee);
uint8_t parseRX16(BayEOS &client, XBee &xbee,int rx_panid);

#endif


