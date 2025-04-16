#ifndef BaySIMA7670_h
#define BaySIMA7670_h

#include <BaySIM800.h>
#include <Base64.h>

class BaySIMA7670 : public BaySIM800
{
public:
	BaySIMA7670(HardwareSerial &serial, uint8_t power_pin = 0) : BaySIM800(serial)
	{
		_power_pin = power_pin;
	};

	/**
	 * Send several frames in one post request
	 * returns 0 for success
	 * 1 == No HTTP 200
	 * 2 == No connect to network
	 * 3 == no connect to server
	 * 4 == could not send data to moden
	 * 6 and more == result code of init()+5
	 */
	uint8_t sendMultiFromBuffer(uint16_t maxsize = 5000, bool ack_payload = false);
	uint8_t sendPayloadWithAck(bool ack_payload = true);
	uint8_t sendPayload(void);

	/**
	 * Switch on GPRS-Modem
	 * 0 == GPRS-modem is up and responding OK
	 * 1 == Communication-ERROR
	 * 2 == PIN failed
	 * 3 == PIN locked
	 * 4 == No CPIN READY
	 * 5 == Not CREG
	 * 6 == Not CGATT
	 * 7 == No SIM Card
	 *
	 * if unlock_only is set, function returns already after unlocking the modem
	 */
	uint8_t begin(long baud);
	uint8_t init(void);

	/**
	 * Checks if modem is registered to network
	 */
	uint8_t isRegistered(void);

	/**
	 * Checks if modem is attached to network
	 */
	uint8_t isAttached(void);

	/**
	 * Disconnect from the web
	 */
	uint8_t getRSSI(void);

protected:
	/**
	 * Helper functions for different Post requests
	 */
	uint8_t postHeader(uint16_t size);
	uint8_t post(void);
	void readAck(void);
	uint8_t _power_pin;
};

#endif
