/**
 * BayEOSBuffer library
 * 
 * holzheu@bayceer.uni-bayreuth.de
 *
 *
 * Data-Format of BayEOS-Buffer
 * [4 byte Time][length][length byte data]...
 * time is either in milliseconds or in seconds - depending on whether a RTC is set!
 *
 *
 *
 *
 */


#ifndef BayEOSBuffer_h
#define BayEOSBuffer_h
#define __PROG_TYPES_COMPAT__ 
#include <Arduino.h>


#define SERIAL_DEBUG 0
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
	 * Read packet to dest but stop at end
	 */
	uint8_t readBinary(unsigned long pos,unsigned long end,uint8_t length, uint8_t *dest);

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
	 * Initialize packet
	 */
	uint8_t initPacket(unsigned long pos);

	/**
	 * Initialize next packet at read pointer
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

	unsigned long writePos(void){ return _write_pos; }
	unsigned long readPos(void){ return _read_pos; }
	unsigned long endPos(void){ return _end; }
	unsigned long length(void){ return _max_length; }


	boolean _absoluteTime;
	boolean _framesDiscarded;

protected:
	unsigned long _max_length;
	RTC* _rtc;
	unsigned long _read_pos;
	unsigned long _write_pos;
	unsigned long _pos;
	unsigned long _end;


private:
	uint8_t b_write(const uint8_t b);
	uint8_t b_write(const uint8_t *b, uint8_t length);
	int b_read();
	int b_read(uint8_t *dest, int length);
	uint8_t b_seek(unsigned long pos);
	/*
	 * reset storage to inital state
	 */
	virtual void resetStorage(void)=0;

	/*
	 * write one byte to the buffer at the current _write_pos
	 */
	virtual uint8_t write(const uint8_t b)=0;

	/*
	 * write length bytes to the buffer at the current _write_pos
	 */
	virtual uint8_t write(const uint8_t *b,uint8_t length)=0;

	/*
	 * seek the write/read pointer of the buffer
	 */
	virtual uint8_t seek(unsigned long pos)=0;

	/*
	 * read one byte from the buffer - retruns -1 on failure
	 */
	virtual int read(void)=0;

	/*
	 * read length bytes from the buffer - retruns -1 on failure
	 */
	virtual int read(uint8_t *dest,int length)=0;


	/*
	 * Flush the buffer
	 */
	virtual void flush(void)=0;

	unsigned long _millis;
	uint8_t _packet_length;
	int _res;

};






#endif
