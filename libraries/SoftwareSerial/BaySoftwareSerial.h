/**
 * Arduino BaySoftwareSerial
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

#ifndef BaySoftwareSerial_h
#define BaySoftwareSerial_h

#include <BaySerial.h>
#include <SoftwareSerial.h>

class BaySoftwareSerial : public SoftwareSerial, public BaySerialInterface {
public:
	/**
	 * Constructor
	 */
	BaySoftwareSerial(uint8_t rxPin, uint8_t txPin ,int timeout=10000);

	int available(void);
	int i_available(void);
	void begin(long baud);
	void flush(void);
	void end(void);
	int read(void);
	size_t write(uint8_t c);

};


#endif
