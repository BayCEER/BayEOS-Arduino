/*
 * Serial Debugging Class
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
 *
 * This is a Class sending BayEOS-Frames in a human readable form
 * helps debugging Arduino-BayEOS-Projects...
 *
 */

#ifndef BayDEBUG_h
#define BayDEBUG_h

#include <BayEOS.h>

class BayEOSDebugInterface :  virtual public Stream, public BayEOS {
public:
    using Print::write; // pull in write(str) and write(buf, size) from Print
	uint8_t sendPayload(void);
protected:
	uint8_t _modus;
	uint8_t _error_next;
private:
	void parse(uint8_t offset=0);
	void parseDataFrame(uint8_t offset);


};

class BayDebug : public BayEOSDebugInterface {
public:
	/**
	 * Constructor
	 */
	BayDebug(HardwareSerial &serial=Serial);


	/**
	 * Begin Serial connection...
	 * modus=0 -> Print out Hex
	 * modus=1 -> Print out frames in human readable form
	 * modus=2 -> Print out Hex with send error every second frame
	 * modus=3 -> Print out frames in human readable form
	 *            with send error every second frame
	 *
	 **/
	void begin(long baud,uint8_t modus=3);
private:
	HardwareSerial* _serial;
	int available(void){return _serial->available();}
	int read(void){return _serial->read();}
	size_t write(uint8_t b){return _serial->write(b);}
    int peek(void){return _serial->peek();};
    void flush(void){_serial->flush();};


};





#endif
