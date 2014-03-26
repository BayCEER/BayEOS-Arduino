#include "BayEOSBuffer.h"


BayEOSBuffer::BayEOSBuffer(void){
#if SERIAL_DEBUG
//	Serial.println("BayEOSBuffer");
#endif
	_rtc=NULL;
}

void BayEOSBuffer::setReadPointer(uint8_t type){
  switch(type){
  case 0:
    _read_pos=0;
    break;
  case 1:
    _read_pos=_write_pos;
    break;
  }
}


unsigned long BayEOSBuffer::available(void){
	return (_end-_read_pos);
}

uint8_t BayEOSBuffer::freeSpace(uint8_t length){
	unsigned long expected_write_pos=_write_pos;
	if((_write_pos+length)>_max_length) expected_write_pos=0;
	return !(expected_write_pos<=_read_pos &&
    _end>_read_pos &&
    (expected_write_pos+length)>_read_pos &&
    length<=_max_length);
}

int BayEOSBuffer::readPacket(uint8_t *dest){
	return read(dest,_packet_length);
}

uint8_t BayEOSBuffer::readBinary(unsigned long pos, uint8_t length, uint8_t *dest){
	seek(pos);
	return read(dest,length);
}


void BayEOSBuffer::set(unsigned long pos){
#if SERIAL_DEBUG
		Serial.print("Set: ");
		Serial.println(pos);
#endif

	_read_pos=pos;
	_write_pos=pos;
	_end=pos;
#if SERIAL_DEBUG
	   Serial.print(" --end: ");
	   Serial.print(_end);
	   Serial.print(" -- write: ");
	   Serial.print(_write_pos);
	   Serial.print(" -- read: ");
	   Serial.println(_read_pos);
#endif
}

void BayEOSBuffer::seekReadPointer(unsigned long pos){
	_read_pos=pos;
}

void BayEOSBuffer::reset(void){
	set(0);
	resetStorage();
}

uint8_t BayEOSBuffer::initNextPacket(void){
	uint8_t* p=(uint8_t*) &_millis;
	seek(_read_pos);
	read(p,4);
	_packet_length=read();
#if SERIAL_DEBUG
		Serial.println("init Packet: ");
	   Serial.print("length: ");
	   Serial.print(_packet_length);
	   Serial.print(" --end: ");
	   Serial.print(_end);
	   Serial.print(" -- write: ");
	   Serial.print(_write_pos);
	   Serial.print(" -- read: ");
	   Serial.println(_read_pos);
#endif
	return _packet_length;

}

void BayEOSBuffer::next(void){
	if(_packet_length==0){
#if SERIAL_DEBUG
		Serial.println("Packet Length is 0. This should not happen...");
#endif
		_read_pos=0;
	} else {
		_read_pos+=_packet_length+5;
#if SERIAL_DEBUG
		Serial.println("next(): Set _read_pos");
#endif
	}

	if(_read_pos>=_end && _write_pos<_end){
		_read_pos=0;
		_end=_write_pos;
#if SERIAL_DEBUG
		Serial.println("Setze Lesezeiger auf 0");
#endif
	}
#if SERIAL_DEBUG
		Serial.print("Next Packet: ");
	   Serial.print("length: ");
	   Serial.print(_packet_length);
	   Serial.print(" --end: ");
	   Serial.print(_end);
	   Serial.print(" -- write: ");
	   Serial.print(_write_pos);
	   Serial.print(" -- read: ");
	   Serial.println(_read_pos);
#endif
}

uint8_t BayEOSBuffer::addPacket(const uint8_t *payload,uint8_t length){
	unsigned long write_new=_write_pos+5+length;
	if(write_new>_max_length){
		//Ende erricht
#if SERIAL_DEBUG
		Serial.println("Ende ereicht");
#endif
		if(_read_pos>=_write_pos ){
			_read_pos=0;
			_end=0;
#if SERIAL_DEBUG
		Serial.println("Reset Lese- + Endzeiger...");
#endif

		}
		_write_pos=0;
		write_new=5+length;
	}


	while(! freeSpace(length)){
		//Write Pointer überholt Read Pointer
		initNextPacket();
		next();
#if SERIAL_DEBUG
		Serial.println("Verwerfe Paket");
#endif
	}

	seek(_write_pos);

	unsigned long time=getTime();
	uint8_t* b=(uint8_t *) &time;
	uint8_t count=0;
//	Serial.println(count);
	count+=write(b,4);
//	Serial.println(count);
	count+=write(length);
//	Serial.println(count);
	count+=write(payload,length);
//	Serial.println(count);
	flush();

	_write_pos+=count;
	if(_end<_write_pos) _end=_write_pos;
#if SERIAL_DEBUG
	   Serial.println();
	   Serial.print("end: ");
	   Serial.print(_end);
	   Serial.print(" -- write: ");
	   Serial.print(_write_pos);
	   Serial.print(" -- read: ");
	   Serial.println(_read_pos);
	   Serial.println();
#endif

	return count;
}



//PROGMEM prog_uint8_t daysInMonth[]  = {31,28,31,30,31,30,31,31,30,31,30,31};

static uint8_t daysInMonth[]={31,28,31,30,31,30,31,31,30,31,30,31};

// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += daysInMonth[(i - 1)];
//    days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second

DateTime::DateTime (long t) {
    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ; ++yOff) {
        leap = yOff % 4 == 0;
        if (days < (uint16_t)365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = daysInMonth[(m - 1)]; //pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    if (year >= 2000)
        year -= 2000;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

// A convenient constructor for using "the compiler's time":
// DateTime now (__DATE__, __TIME__);
// NOTE: using PSTR would further reduce the RAM footprint
DateTime::DateTime (const char* date, const char* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0]) {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
   
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

uint8_t DateTime::dayOfWeek() const {
    uint16_t day = get() / SECONDS_PER_DAY;
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

long DateTime::get() const {
    uint16_t days = date2days(yOff, m, d);
    return time2long(days, hh, mm, ss);
}

