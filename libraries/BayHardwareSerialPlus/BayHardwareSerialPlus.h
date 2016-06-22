#ifndef BayTCPSIM900Plus_h
#define BayTCPSIM900Plus_h

#define SIM900_DEBUG 0


#include <HardwareSerialPlus.h>
#include <BayTCPSim900.h>
#include <BayXBee.h>
#include <BaySerial.h>
#include <BayDebug.h>
#include "../BayTCP/BayTCP.h"
#include "../BayRTClib/RTClib.h"

class BayGPRSPlus : public BayGPRSInterface {
public:
	/**
	 * Constructor
	 */
	BayGPRSPlus(HardwareSerialPlus &serial=SerialPlus, uint8_t powerPin=9,uint8_t resetPin=0){
		_powerPin=powerPin;
		_urlencode=1;
		_resetPin=resetPin;
		_serial=&serial;
	}
	uint8_t begin(long baud){
		_baud=baud;
		return init();
	}
private:
	HardwareSerialPlus* _serial; //Pointer to existing serial object!!
	int available(void){return _serial->available();}
	int read(void){
#if SIM900_DEBUG
		int c=_serial->read();
		if(c!=-1) SerialPlus.write(c);
		return c;
#else
		return _serial->read();
#endif

	}
	void i_begin(long b){ _serial->begin(b);}
	int i_available(void){return _serial->available();}
	size_t write(uint8_t b){
#if SIM900_DEBUG
		SerialPlus.write(b);
#endif
		return _serial->write(b);
	}
    int peek(void){return _serial->peek();};
    void flush(void){_serial->flush();};
};


class BayXBeePlus: public BayXBeeInterface {
private:
	HardwareSerialPlus* _serial;

public:
	BayXBeePlus(uint8_t sleep_pin=0, uint8_t wakeup_time=15, int wait_time_for_response=5000,uint16_t dest=BAYXBEE_GATEWAY):
		BayXBeeInterface(sleep_pin,wakeup_time,wait_time_for_response,dest)
	{
		_serial = &SerialPlus;
	}

	void setSerial(HardwareSerialPlus &serial){
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

class XBeePlus : public XBeeInterface {
private:
	HardwareSerialPlus* _serial;

public:
	XBeePlus(HardwareSerialPlus &serial=SerialPlus):XBeeInterface()
	{
		_serial = &serial;
	}

	void setSerial(HardwareSerialPlus &serial){
		_serial = &serial;
	}

	int i_available(void){
		return _serial->available();
	}
	void begin(long baud){
		_serial->begin(baud);
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

class BaySerialPlus : public BaySerialInterface {
private:
	HardwareSerialPlus* _serial; //Pointer to existing serial object!!
public:
	/**
	 * Constructor
	 */
	BaySerialPlus(HardwareSerialPlus& serial=SerialPlus,int timeout=1000){
		_serial=&serial;
		BaySerialInterface::_timeout=timeout;
	}

	int available(void){
		return _serial->available();
	}
	int i_available(void){
		return _serial->available();
	}
	void begin(long baud){
		_serial->begin(baud);
	}
	void flush(void){
		_serial->flush();
	}
	void end(void){
		_serial->end();
	}
	int read(void){
		return _serial->read();
	}

	size_t write(uint8_t c){
		return _serial->write(c);
	}

};

class BayDebugPlus : public BayEOSDebugInterface {
private:
	HardwareSerialPlus* _serial; //Pointer to existing serial object!!
public:
	/**
	 * Constructor
	 */
	BayDebugPlus(HardwareSerialPlus& serial=SerialPlus){
		_serial=&serial;
	}

	int available(void){
		return _serial->available();
	}
	int i_available(void){
		return _serial->available();
	}
	void begin(long baud, uint8_t modus=0){
		_serial->begin(baud);
	}
	void flush(void){
		_serial->flush();
	}
	void end(void){
		_serial->end();
	}
	int read(void){
		return _serial->read();
	}

	size_t write(uint8_t c){
		return _serial->write(c);
	}

};





#endif
