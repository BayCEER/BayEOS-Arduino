/*
 * This is a Arduino Library for
 * FTLab's RD200M Radon Sensor
 *
 * it is inspired by
 * https://github.com/ekyuho/sensors
 *
 * Serial Connection can be established via Hard- or Software-Serial
 *
 */


#ifndef RD200M_H
#define RD200M_H
#include <Arduino.h>
#include <SoftwareSerial.h>


class RD200M_interface : public Stream{
public:
    void update(void);
    void reset(void);
    void init(void);
    float _value;
    uint8_t _elapsed;
    uint8_t _up;
    uint8_t _status;
    bool _ready;

protected:
    using Print::write; // pull in write(str) and write(buf, size) from Print
    virtual int i_available(void);
    virtual void begin(long b);
private:
    byte req[4] = { 0x02, 0x01, 0x00, 0xFE };
    byte _reset[4] = { 0x02, 0xA0, 0x00, 0xFF-0xA0 };
    int _csum;
    int _state;

};

class RD200M : public RD200M_interface {
public:
	RD200M(HardwareSerial &serial);
private:
	HardwareSerial* _serial; //Pointer to existing serial object!!
	int i_available(void){return _serial->available();}
	int available(void){return _serial->available();}
	int read(void){	return _serial->read();	}
	void begin(long b){ _serial->begin(b);}
	void end(void){ _serial->end();}
	size_t write(uint8_t b){return _serial->write(b);}
    int peek(void){return _serial->peek();};
    void flush(void){_serial->flush();};
};

class RD200MSoftserial : private SoftwareSerial, public RD200M_interface {
public:
	/**
	 * Constructor
	 */
	RD200MSoftserial(uint8_t rxPin, uint8_t txPin);
private:
	int i_available(void){return SoftwareSerial::available();}
	int read(void){	return SoftwareSerial::read(); }
	void begin(long b){ SoftwareSerial::begin(b);}
	void end(void){ SoftwareSerial::end();}
	int available(void){return SoftwareSerial::available();}
	size_t write(uint8_t b){return SoftwareSerial::write(b);}
    int peek(void){return SoftwareSerial::peek();};
    void flush(void){SoftwareSerial::flush();};
};

#endif
