#ifndef BaySIM800_h
#define BaySIM800_h

#define BaySIM800_BUFFER 138


#define printP(x) printPGM(PSTR(x))
#define printlnP(x) printlnPGM(PSTR(x))
#define printlnP_OK(x,y) printlnPGM(PSTR(x),y)
#define wait_for(x,y) wait_forPGM(PSTR(x),y)


#include <BayEOS.h>
#include <Base64.h>

class BaySIM800 :  public BayEOS {
public:
	BaySIM800(HardwareSerial& serial);

	/**
	 * read config from string - use | as field delimiter
	 * url|user|password|Sender|APN|APN-USER|APN-PW|PIN|
	 *
	 */
	void readConfigFromStringPGM(const char* string);


	/**
	 * Send several frames in one post request
	 * returns 0 for success
	 * 1 == No HTTP 200
	 * 2 == No connect to network
	 * 3 == no connect to server
	 * 4 == could not send data to moden
	 * 6 and more == result code of init()+5
	 */
	uint8_t sendMultiFromBuffer(uint16_t maxsize=5000,bool ack_payload=false);
	uint8_t sendPayloadWithAck(bool ack_payload=true);
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
	 * Disconnect from the web
	 */
	uint8_t getRSSI(void);

	/**
	 * Get time from RTC
	 */
	DateTime now(void);

protected:
	/**
	 * Helper functions for different Post requests
	*/
	uint8_t postHeader(uint16_t size);
	uint8_t post(void);
	void readAck(void);



	HardwareSerial* _serial; //Pointer to existing serial object!!
    long _baud;

	void skipChars(void);
	uint8_t wait_forOK(uint16_t timeout);
	uint8_t wait_forPGM(const char* str, uint16_t timeout,uint8_t bytes=0, char* buffer=NULL);
	uint8_t wait_for_available(int bytes=1);
	void printPGM(const char *str);
	void printlnPGM(const char *str);
	uint8_t printlnPGM(const char *str, uint16_t timeout);
	void urlDecode(char *str);
	uint8_t strlenURLencoded(const char *str);
	void printURLencoded(const char *str);
	uint8_t URLencoded(char c,bool print=true);

	uint8_t getConfig(const char * config,bool print=true,bool urlencoded=false);

	const char* _url; //0
	const char* _user; //1
	const char* _password; //2
	const char* _sender; //3
	const char* _apn; //4
	const char* _prov_user; //5
	const char* _prov_pw; //6
	const char* _pin; //7
	char _base64buffer[BaySIM800_BUFFER];
	uint8_t _tx_error_count;
	uint16_t _mtu;
	uint16_t _timeout;
	static const char* const _urlencodedChars;

};


#endif
