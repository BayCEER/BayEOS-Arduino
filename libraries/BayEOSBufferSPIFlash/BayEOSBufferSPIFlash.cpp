#include "BayEOSBufferSPIFlash.h"
#define SERIAL_DEBUG 0
BayEOSBufferSPIFlash::BayEOSBufferSPIFlash() :
		BayEOSBuffer() {
}

void BayEOSBufferSPIFlash::init(SPIFlash& flash, uint8_t flush_skip) {
	_flash = &flash;
	_flash->begin();
	_flush_skip = flush_skip;
	_flush_count = 0;
	_max_length = _flash->getCapacity() - 4096; //we reserve the last sector for pointer storage information
	uint8_t *p;
	p = (uint8_t*) &_temp;
	_flush_offset = 0;
	_flash->readByteArray(_max_length + 16 * _flush_offset, p, 4);
	while (_temp == 0x0f0f0f0fL && _flush_offset < 255) {
		_flush_offset++;
		_flash->readByteArray(_max_length + 16 * _flush_offset, p, 4);
#if SERIAL_DEBUG
		Serial.println("Restoring pointers...");
#endif
	}

	if (_flush_offset > 0) {
		//Restore pointers
		_flash->readByteArray(_max_length + 16 * _flush_offset - 12, p, 4);
		if (_temp < _max_length)
			_read_pos = _temp;
		_flash->readByteArray(_max_length + 16 * _flush_offset - 8, p, 4);
		if (_temp < _max_length)
			_write_pos = _temp;
		_flash->readByteArray(_max_length + 16 * _flush_offset - 4, p, 4);
		if (_temp < _max_length)
			_end = _temp;

		//Move ahead until we find 0xffffffff or reach 255
		uint8_t i=255;
		while (i) {
			i--;
			_flash->readByteArray(_write_pos, p, 4);
			uint8_t l = _flash->readByte(_write_pos + 4);
			uint8_t move_read_pos = 0;

			if (_temp == 0xffffffff)
				break;
			while (!freeSpace(l + 5)) {
				//Write Pointer überholt END Pointer
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

			_write_pos += l + 5;
#if SERIAL_DEBUG
			Serial.print(_write_pos);
			Serial.print("\t");
			Serial.println(_end);
			Serial.flush();
#endif
		}
	} else {
		reset();
	}

}

void BayEOSBufferSPIFlash::resetStorage(void) {
	uint8_t tries=0;

	eraseSector(0); //only erase Sector 0
	eraseSector(_max_length); //erase pointer region
}

void BayEOSBufferSPIFlash::checkEraseSector(const unsigned long start_pos,
		unsigned long end_pos) {
	if ((start_pos >> 12) != (end_pos >> 12)) {
		uint8_t move_read_pos=0;
		if (end_pos >= _max_length)
			end_pos = 0;
		while (_end >= end_pos && _end < (end_pos+4096)) {
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
		eraseSector(end_pos);

		_flush_count=_flush_skip;
		b_seek(start_pos);
	}

}

uint8_t BayEOSBufferSPIFlash::write(const uint8_t b) {
#if SERIAL_DEBUG
	Serial.print("W");
	Serial.println(_pos);
#endif
	if (_pos < _max_length) {
		checkEraseSector(_pos, _pos + 1);
		return write_flash(_pos,&b,1);
	}
	return 0;
}
uint8_t BayEOSBufferSPIFlash::write(const uint8_t *b, uint8_t length) {
#if SERIAL_DEBUG
	Serial.print("W");
	Serial.print(_pos);
	Serial.print("-");
	Serial.println(_pos+length);
#endif
	if (_pos + length <= _max_length) {
		checkEraseSector(_pos, _pos + length);
		return write_flash(_pos, b, length);
	}
	return 0;
}

uint8_t BayEOSBufferSPIFlash::write_flash(unsigned long pos,const uint8_t *b, uint8_t length) {
	uint8_t tries=0;
	while(! _flash->writeByteArray(pos, b, length, true)){
		delay(10);
		tries++;
		if(tries>2){
#if SERIAL_DEBUG
	Serial.println("Write flash failed");
#endif
			return 0;
		}
	}
	return length;
}

uint8_t BayEOSBufferSPIFlash::seek(unsigned long pos) {
	if (pos < _max_length) {
//		_pos = pos; //done in b_seek
		return true;
	} else
		return false;
}

int BayEOSBufferSPIFlash::read(void) {
	uint8_t data = 0;
	read_flash(_pos,&data,1);
	return data;
}

int BayEOSBufferSPIFlash::read(uint8_t *dest, int length) {
	return read_flash(_pos,dest,length);
}

int BayEOSBufferSPIFlash::read_flash(unsigned long pos, uint8_t *dest,int length){
	uint8_t tries=0;
	while(! _flash->readByteArray(pos, dest, length)){
		delay(10);
		tries++;
		if(tries>2) return 0;
	}
	return length;
}

bool BayEOSBufferSPIFlash::eraseSector(unsigned long pos){
#if SERIAL_DEBUG
		Serial.print("Erase Sector ");
		Serial.println(pos);
#endif
	uint8_t tries;
	while(! _flash->eraseSector(pos)){
		delay(10);
		tries++;
		if(tries>10){
#if SERIAL_DEBUG
		Serial.println("Erase Sector failed...");
#endif
			return false;
		}
	}
	return true;
}

void BayEOSBufferSPIFlash::flush(void) {
	_flush_count++;
	// make sure that there is always 0xffffffff left between end of write and end of buffer
	checkEraseSector(_write_pos,_write_pos+4);
#if SERIAL_DEBUG
	Serial.print("Flush (");
	Serial.print(_flush_offset);
	Serial.print(") ");
#endif
	if (_flush_count < _flush_skip)
		return;
	//save the pointers to EEPROM
	if (_flush_offset == 255) {
		_flush_offset = 0;
	}
	if (_flush_offset == 0)
		eraseSector(_max_length);

#if SERIAL_DEBUG
	Serial.print("Flush");
#endif
	_temp = 0x0f0f0f0fL;
	uint8_t *p;
	p = (uint8_t*) &_temp;
	write_flash(_max_length + 16 * _flush_offset, p, 4);
	p = (uint8_t*) &_read_pos;
	write_flash(_max_length + 16 * _flush_offset + 4,p,4);
	p = (uint8_t*) &_write_pos;
	write_flash(_max_length + 16 * _flush_offset + 8,p,4);
	p = (uint8_t*) &_end;
	write_flash(_max_length + 16 * _flush_offset + 12,p,4);
	_flush_offset++;
	_flush_count = 0;
#if SERIAL_DEBUG
	Serial.println(" done");
#endif
}

