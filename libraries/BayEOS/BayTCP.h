/**
 * Arduino BayTCP
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
 *
 * Base-Class for sending HTTP-Post-Request to a BayEOS Gateway
 *
 */

#ifndef BayTCP_h
#define BayTCP_h


#ifndef BayTCP_BUFFER
#define BayTCP_BUFFER 138
#endif

#define BayTCP_CONFIG_SERVER 0
#define BayTCP_CONFIG_PORT 1
#define BayTCP_CONFIG_PATH 2
#define BayTCP_CONFIG_USER 3
#define BayTCP_CONFIG_PASSWORD 4
#define BayTCP_CONFIG_SENDER 5
#define BayTCP_CONFIG_APN 6
#define BayTCP_CONFIG_PROVPW 7
#define BayTCP_CONFIG_PROVUSER 8
#define BayTCP_CONFIG_PIN 9
#define BayTCP_CONFIG_MAC 6
#define BayTCP_CONFIG_IP 7
#define BayTCP_CONFIG_MASK 8
#define BayTCP_CONFIG_DEFAULT_GW 9

#define BayTCP_CONFIG_SIZE 140
#define BayTCP_EEPROM_OFFSET 400

#define BayTCP_DEBUG_OUTPUT 0
#define BayTCP_DEBUG_INPUT 0

#define BayTCP_DEBUG_INTERFACE Serial

#define printP(x) printPGM(PSTR(x))
#define printlnP(x) printlnPGM(PSTR(x))
#define wait_for(x,y) wait_forPGM(PSTR(x),y)
#define SerialprintP(x) serialprintPGM(PSTR(x))
#define SerialprintlnP(x) serialprintlnPGM(PSTR(x))


#include <inttypes.h>
#include <BayEOS.h>
#include "Base64.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "../BayEOSBuffer/BayEOSBuffer.h"


class BayTCPInterface : public Stream, public BayEOS {
protected:
//	virtual uint8_t i_init() = 0;
	virtual int i_available() = 0;
public:
    using Print::write; // pull in write(str) and write(buf, size) from Print
	/**
	 * Send the BayEOS payload as base64 encoded string
	 * returns 0 for success
	 * 1 == no success
	 * 2 == timeout
	 * 3 and more == result code of connect()+2
	 */
	uint8_t sendPayload(void);


	/**
	 * Connect to the web through apn
	 * 0 == OK
	 * 1 == Network TIMEOUT
	 * 2 == Device TIMEOUT
	 * 3 and more == specific result codes of implementation
	 */
	virtual uint8_t connect(void)=0;

	/**
	 * Disconnect from the web
	 */
	virtual void disconnect(void)=0;




	/**
	 * Send several frames in one post request
	 * returns 0 for success
	 * 1 == no success
	 * 2 == timeout
	 * 3 and more == result code of connect()+2
	 */
	uint8_t sendMultiFromBuffer(uint16_t maxsize=5000);
	uint8_t sendMultiFromBufferWithAckPayload(uint16_t maxsize=5000);

	/**
	 * set as config value in config buffer
	 */
	void setConfig(const char *str,uint8_t index);

	/**
	 * read config from string - use | as field delimiter
	 * 132.180.112.x|80|path|user|password|Sender|APN|APN-PW|APN-USER|PIN|
	 *
	 */
	void readConfigFromStringPGM(const char* string);

	/**
	 * read config from EEPROM
	 */
	void readConfigFromEEPROM(int eeoffset=BayTCP_EEPROM_OFFSET);
	/**
	 * write config to EEPROM
	 */
	void writeConfigToEEPROM(int eeoffset=BayTCP_EEPROM_OFFSET);

	/**
	 * get pointer to config buffer
	 * server\0port\0path\0user\0\0password\0sender\0apn\0apn_pw\0apn_user\0\pin\0
	 */
	const char* getConfig(void);

	const uint8_t* parseMAC(const char* str);
	const uint8_t* parseIP(const char* str);
	uint8_t hex2int(const char c);

	/**
	 * get a pointer to a specific config buffer entry
	 * e.g Serial.print(*getConfigPointer(APN));
	 */
	char** getConfigPointer(uint8_t index);

	/**
	 * URL encode POST request. Default true
	 */
	boolean _urlencode;
protected:
	virtual void flushMTU(void)=0;
	virtual void finishTransmissionMode(void)=0;
	void printPostHeader(uint16_t size);
	void setConfigPointers(void);
	uint8_t addToConfigBuffer(uint8_t offset,const char* str);

	void skipChars(void);
	uint8_t wait_forOK(uint16_t timeout);
	uint8_t wait_forPGM(const char* str, uint16_t timeout,uint8_t bytes=0, char* buffer=NULL);
	uint8_t wait_for_available(uint16_t* timeout,int bytes=1);
	void printPGM(const char *str);
	void printlnPGM(const char *str);
	void serialprintPGM(const char *str);
	void serialprintlnPGM(const char *str);

	uint8_t _mac_ip[6];
	char _config_buffer[BayTCP_CONFIG_SIZE];
//	char _pgm_buffer[20];
	char* _server; //0
	char* _port; //1
	char* _path; //2
	char* _user; //3
	char* _password; //4
	char* _sender; //5
	char* _apn; //6
	char* _prov_pw; //7
	char* _prov_user; //8
	char* _pin; //9
	char _base64buffer[BayTCP_BUFFER];
	uint8_t _tx_error_count;
	uint16_t _mtu;
	void urlDecode(char *str);
	int strlenURLencoded(const char *str);
	void printURLencoded(const char *str);
	static const char* const _urlencodedChars;

};






#endif
