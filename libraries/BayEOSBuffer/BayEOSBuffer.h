/**
 * BayEOSBuffer library
 * 
 * holzheu@bayceer.uni-bayreuth.de
 *
 *
 */


#ifndef BayEOSBuffer_h
#define BayEOSBuffer_h
#define __PROG_TYPES_COMPAT__ 
#include <Arduino.h>


#ifndef SERIAL_DEBUG
#define SERIAL_DEBUG 0
#endif
//RTC-Stuff...
#define SECONDS_PER_DAY 86400L


class DateTime {
public:
    DateTime (long t =0);
    DateTime (uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour =0, uint8_t min =0, uint8_t sec =0);
    DateTime (const char* date, const char* time);

    uint16_t year() const       { return 2000 + yOff; }
    uint8_t month() const       { return m; }
    uint8_t day() const         { return d; }
    uint8_t hour() const        { return hh; }
    uint8_t minute() const      { return mm; }
    uint8_t second() const      { return ss; }
    uint8_t dayOfWeek() const;

    // 32-bit times as seconds since 1/1/2000
    long get() const;

protected:
    uint8_t yOff, m, d, hh, mm, ss;
};

//Abstract RTC
class RTC {
public:
	virtual void begin()=0;
	virtual void adjust(const DateTime& dt)=0;
	virtual DateTime now()=0;

};





class BayEOSBuffer {
public:
	BayEOSBuffer(void);


	/**
	 * Check if there are any bytes available for reading from the buffer.
	 * returns number of bytes available for reed
	 */
	unsigned long available(void);

	/**
	 * Checks if there is "length" space left
	 * return 0 if not
	 */
	uint8_t freeSpace(uint8_t length);


	/**
	 * Read packet to dest
	 */
	int readPacket(uint8_t *dest);

	/**
	 * Read packet to dest
	 */
	uint8_t readBinary(unsigned long pos,uint8_t length, uint8_t *dest);

	/**
	 * Set read pointer to next packet
	 */
	void next(void);

	/**
	 * Set all pointers to pos
	 */
	void set(unsigned long pos);
	/**
	 * Set read pointer to pos
	 */
	void seekReadPointer(unsigned long pos);
	/**
	 * reset buffer to zero
	 */
	void reset(void);
	/**
	 * Add BayEOS frame packet to buffer
	 * returns number of bytes written
	 */
	uint8_t addPacket(const uint8_t *payload,uint8_t length);

	/**
	 * Initialize next package
	 */
	uint8_t initNextPacket(void);
	/**
	 * Get length of packet (call after initNextPacket()
	 */
	uint8_t packetLength(void){
		return _packet_length;
	}
	/**
	 * Get millis of packet (call after initNextPacket()
	 */
	unsigned long packetMillis(void){
		return _millis;
	}


	/**
	 * set rtc pointer to a RTC instance
	 * NOTE: setting absolute_time to false will result in
	 * relative time delayed frames...
	 */
	void setRTC(RTC& rtc,boolean absolute_time=true) {
		_rtc = &rtc;
		_absoluteTime=absolute_time;
	}

	/**
	 * check existance of rtc
	 * return true if there is a rtc
	 */
	uint8_t rtc(void){ 
	  if(_rtc!=NULL) return 1;
	  return 0;
	}


	unsigned long getTime(void){
	  if(_rtc!=NULL){
		//DateTime now=_rtc->now();
		return _rtc->now().get();
	  }
	  return millis(); 
	}
	/**
	 * Set read pointer to 
	 * 0 = Start of buffer
	 * 1 = writePointer
	 */
	void setReadPointer(uint8_t type);

	unsigned long writePos(void){ return _write_pos; }
	unsigned long readPos(void){ return _read_pos; }


	boolean _absoluteTime;

protected:
	unsigned long _max_length;
	RTC* _rtc;


private:

	virtual void resetStorage(void)=0;
	virtual uint8_t write(const uint8_t b)=0;
	virtual uint8_t write(const uint8_t *b,uint8_t length)=0;
	virtual uint8_t seek(unsigned long pos)=0;
	virtual int read(void)=0;
	virtual int read(uint8_t *dest,int length)=0;
	virtual void flush(void)=0;

	unsigned long _read_pos;
	unsigned long _write_pos;
	unsigned long _end;
	unsigned long _millis;
	uint8_t _packet_length;

};






#endif
