#include "BayEOSBuffer.h"

BayEOSBuffer::BayEOSBuffer(void) {
#if SERIAL_DEBUG
//	Serial.println("BayEOSBuffer");
#endif
	_rtc = NULL;
	_framesDiscarded = 0;
}

uint8_t BayEOSBuffer::b_seek(unsigned long pos) {
	_pos = pos;
	return seek(pos);
}
uint8_t BayEOSBuffer::b_write(const uint8_t b) {
	_res = write(b);
	_pos++;
	if (_pos == _max_length) {
		b_seek(0);
	}
	return _res;

}
uint8_t BayEOSBuffer::b_write(const uint8_t *b, uint8_t length) {
	//calculate length for first write
	_res = (((unsigned long) (_pos + length)) <= _max_length ?
			length : _max_length - _pos);
	_res = write(b, _res);
	if (_res < 0)
		return 0; //error!!
	_pos += _res;
	if (_pos == _max_length) {
		b_seek(0);
	}
	if (_res < length) {
		//second write
		_res = write(b + _res, length - _res);
		if (_res < 0)
			return 0; //error!!
		_pos += _res;
		_res = length;
	}
	return _res;

}

int BayEOSBuffer::b_read() {
	_res = read();
	_pos++;
	if (_pos == _max_length) {
		b_seek(0);
	}
	return _res;
}

int BayEOSBuffer::b_read(uint8_t *dest, int length) {
	//calculate length for first read
	_res = (((unsigned long) (_pos + length)) <= _max_length ?
			length : _max_length - _pos);
	_res = read(dest, _res);
	if (_res == -1)
		return -1;
	_pos += _res;
	if (_pos == _max_length) {
		b_seek(0);
	}
	if (_res < length) {
		//second read
		_res = read(dest + _res, length - _res);
		if (_res == -1)
			return -1;
		_pos = _res;
		_res = length;
	}
	return _res;
}

unsigned long BayEOSBuffer::available(void) {
	return ((unsigned long) (_max_length + _write_pos - _read_pos))
			% _max_length;
}

unsigned long BayEOSBuffer::freeSpace(void) {
	if (_read_pos == _write_pos)
		return _max_length;
	return (((unsigned long) (_max_length + _read_pos - _write_pos))
			% _max_length);
}

void BayEOSBuffer::makeFreeSpace(uint16_t length) {
	uint8_t move_read_pos = 0;
	while (!freeSpace(length + 5)) {
		//End Pointer überholt Read Pointer
		if (_end == _read_pos) {
			_framesDiscarded = 1;
			move_read_pos = 1;
#if SERIAL_DEBUG
			Serial.println("Verwerfe Paket");
#endif
		}
		_end += initPacket(_end) + 5;
		if (_end >= _max_length)
			_end -= _max_length;
		if (move_read_pos)
			_read_pos = _end;
	}

}

uint8_t BayEOSBuffer::freeSpace(uint16_t length) {
	if (_end == _write_pos)
		return (_max_length > (unsigned long) length);
#if SERIAL_DEBUG
	Serial.print("Space: ");
	Serial.print(length);
	Serial.print(" ");
	Serial.print((((unsigned long) (_max_length + _end - _write_pos)) % _max_length));
	Serial.print(" Free: ");
	Serial.println((((unsigned long) (_max_length + _end - _write_pos)) % _max_length)> (unsigned long) length);
#endif
	return (((unsigned long) (_max_length + _end - _write_pos)) % _max_length)
			> (unsigned long) length;
}

int BayEOSBuffer::readPacket(uint8_t *dest) {
	return b_read(dest, _packet_length);
}

uint8_t BayEOSBuffer::readBinary(unsigned long pos, uint8_t length,
		uint8_t *dest) {
	b_seek(pos);
	return b_read(dest, length);
}

uint8_t BayEOSBuffer::readBinary(unsigned long pos, unsigned long stop,
		uint8_t length, uint8_t *dest) {
	b_seek(pos);
	unsigned long remaining = ((unsigned long) (_max_length + stop - pos))
			% _max_length;
	if (remaining < length)
		length = remaining;
	return b_read(dest, length);
}

void BayEOSBuffer::set(unsigned long pos) {
#if SERIAL_DEBUG
	Serial.print("Set: ");
	Serial.println(pos);
#endif

	_read_pos = pos;
	_write_pos = pos;
	_end = pos;
#if SERIAL_DEBUG
	Serial.print(" --end: ");
	Serial.print(_end);
	Serial.print(" -- write: ");
	Serial.print(_write_pos);
	Serial.print(" -- read: ");
	Serial.println(_read_pos);
#endif
}

void BayEOSBuffer::seekReadPointer(unsigned long pos) {
	_read_pos = pos;
}

void BayEOSBuffer::reset(void) {
	set(0);
	resetStorage();
}

void BayEOSBuffer::skip(void) {
	_read_pos = _write_pos;
}

uint8_t BayEOSBuffer::initNextPacket(void) {
	return initPacket(_read_pos);
}

uint8_t BayEOSBuffer::initPacket(unsigned long pos) {
	uint8_t* p = (uint8_t*) &_millis;
	b_seek(pos);
	b_read(p, 4);
	_packet_length = b_read();
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

void BayEOSBuffer::next(void) {
	if (_packet_length == 0
			|| (_read_pos < _write_pos
					&& (_read_pos + 5 + _packet_length) > _write_pos)) {
#if SERIAL_DEBUG
		Serial.println("Packet Length is 0 or invalid!. This should not happen...");
#endif
		_read_pos = _write_pos;
	} else {
		_read_pos += _packet_length + 5;
#if SERIAL_DEBUG
		Serial.println("next(): Set _read_pos");
#endif
	}

	if (_read_pos >= _max_length)
		_read_pos -= _max_length;
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

uint8_t BayEOSBuffer::addPacket(const uint8_t *payload, uint8_t length) {
	makeFreeSpace(length + 5);
	b_seek(_write_pos);
	unsigned long time = getTime();
	uint8_t* b = (uint8_t *) &time;
	bool error = false;
	error=! b_write(b, 4);
	if(! error) error=! b_write(length);
	if(! error) error=! b_write(payload, length);
	if(! error){
		_write_pos	+= length + 5;
		if (_write_pos >= _max_length)
			_write_pos -= _max_length;
		flush();
		return length + 5;		
	} else {//This should never happen
		return 0;
	}
}

uint8_t BayEOSBuffer::packetLength(void) {
	return _packet_length;
}

unsigned long BayEOSBuffer::packetMillis(void) {
	return _millis;
}

void BayEOSBuffer::setRTC(RTC& rtc, uint8_t timeType) {
	_rtc = &rtc;
	_timeType = timeType;
}

uint8_t BayEOSBuffer::rtc(void) {
	if (_rtc != NULL)
		return 1;
	return 0;
}

unsigned long BayEOSBuffer::getTime(void) {
	if (_rtc != NULL) {
		//DateTime now=_rtc->now();
		return _rtc->now().get();
	}
	return millis();
}

unsigned long BayEOSBuffer::writePos(void) {
	return _write_pos;
}
unsigned long BayEOSBuffer::readPos(void) {
	return _read_pos;
}
unsigned long BayEOSBuffer::endPos(void) {
	return _end;
}
unsigned long BayEOSBuffer::length(void) {
	return _max_length;
}

//PROGMEM prog_uint8_t daysInMonth[]  = {31,28,31,30,31,30,31,31,30,31,30,31};

static uint8_t daysInMonth[] =
		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

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

DateTime::DateTime(long t) {
	ss = t % 60;
	t /= 60;
	mm = t % 60;
	t /= 60;
	hh = t % 24;
	uint16_t days = t / 24;
	uint8_t leap;
	for (yOff = 0;; ++yOff) {
		leap = yOff % 4 == 0;
		if (days < (uint16_t) 365 + leap)
			break;
		days -= 365 + leap;
	}
	for (m = 1;; ++m) {
		uint8_t daysPerMonth = daysInMonth[(m - 1)]; //pgm_read_byte(daysInMonth + m - 1);
		if (leap && m == 2)
			++daysPerMonth;
		if (days < daysPerMonth)
			break;
		days -= daysPerMonth;
	}
	d = days + 1;
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
		uint8_t min, uint8_t sec) {
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
DateTime::DateTime(const char* date, const char* time) {
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	yOff = conv2d(date + 9);
	// Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	switch (date[0]) {
	case 'J':
		m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7;
		break;
	case 'F':
		m = 2;
		break;
	case 'A':
		m = date[2] == 'r' ? 4 : 8;
		break;
	case 'M':
		m = date[2] == 'r' ? 3 : 5;
		break;
	case 'S':
		m = 9;
		break;
	case 'O':
		m = 10;
		break;
	case 'N':
		m = 11;
		break;
	case 'D':
		m = 12;
		break;
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

uint16_t DateTime::year() const {
	return 2000 + yOff;
}
uint8_t DateTime::month() const {
	return m;
}
uint8_t DateTime::day() const {
	return d;
}
uint8_t DateTime::hour() const {
	return hh;
}
uint8_t DateTime::minute() const {
	return mm;
}
uint8_t DateTime::second() const {
	return ss;
}

