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
	while (_temp == 0x0f0f0f0fL && _flush_offset < 255){
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

		//We have to erase the current write poiter sector as we do not know whether it has be written something after the pointer
		checkEraseSector(_write_pos, _write_pos + 4096);
		//copy current sector from start to write poiter to following sector
		_pos = _write_pos & 0xfffff000L;
		while (_pos < _write_pos) {
			_flash->writeByte(_pos + 4096, _flash->readByte(_pos));
			_pos++;
		}
		_flash->eraseSector(_write_pos);
		//copy back
		_pos = _write_pos & 0xfffff000L;
		while (_pos < _write_pos) {
			_flash->writeByte(_pos, _flash->readByte(_pos + 4096));
			_pos++;
		}
	} else{
		reset();
	}

}

void BayEOSBufferSPIFlash::resetStorage(void) {
	_flash->eraseSector(0); //only earse Sector 0
	_flash->eraseSector(_max_length); //erase pointer region
}

void BayEOSBufferSPIFlash::checkEraseSector(const unsigned long start_pos,
		unsigned long end_pos) {
	if ((start_pos >> 12) != (end_pos >> 12)) {
		if(end_pos>=_max_length) end_pos=0;
		while (_read_pos >= end_pos && _read_pos < (end_pos + 4096)) {
			initNextPacket();
			next();
			_framesDiscarded = 1;
#if SERIAL_DEBUG
			Serial.println("Frames discarded");
#endif
		}
		_flash->eraseSector(end_pos);
#if SERIAL_DEBUG
		Serial.println("Erase sector");
#endif
		seek(start_pos);
	}

}

uint8_t BayEOSBufferSPIFlash::write(const uint8_t b) {
	if (_pos < _max_length) {
		checkEraseSector(_pos, _pos + 1);
		_flash->writeByte(_pos, b);
		_pos++;
		return 1;

	}
	return 0;
}
uint8_t BayEOSBufferSPIFlash::write(const uint8_t *b, uint8_t length) {
	if (_pos + length < _max_length) {
		checkEraseSector(_pos, _pos + length);
		_flash->writeByteArray(_pos, b, length);
		_pos += length;
		return length;
	}
	return 0;
}

uint8_t BayEOSBufferSPIFlash::seek(unsigned long pos) {
	if (pos < _max_length) {
		_pos = pos;
		return true;
	} else
		return false;
}

int BayEOSBufferSPIFlash::read(void) {
	return _flash->readByte(_pos++);
}

int BayEOSBufferSPIFlash::read(uint8_t *dest, int length) {
	_flash->readByteArray(_pos, dest, length);
	_pos += length;
	return length;
}

void BayEOSBufferSPIFlash::flush(void) {
#if SERIAL_DEBUG
	Serial.print("Flush (");
	Serial.print(_flush_offset);
	Serial.print(") ");
#endif
	_flush_count++;
	if (_flush_count < _flush_skip)
		return;

	//save the poiters to EEPROM
	if (_flush_offset == 255) {
		_flush_offset = 0;
	}
	if(_flush_offset == 0 )
		_flash->eraseSector(_max_length);
#if SERIAL_DEBUG
	Serial.print("Flush");
#endif
	_temp = 0x0f0f0f0fL;
	uint8_t *p;
	p = (uint8_t*) &_temp;
	_flash->writeByteArray(_max_length + 16 * _flush_offset, p, 4);

	p = (uint8_t*) &_read_pos;
	_flash->writeByteArray(_max_length + 16 * _flush_offset + 4, p, 4);
	p = (uint8_t*) &_write_pos;
	_flash->writeByteArray(_max_length + 16 * _flush_offset + 8, p, 4);
	p = (uint8_t*) &_end;
	_flash->writeByteArray(_max_length + 16 * _flush_offset + 12, p, 4);
	_flush_offset++;
	_flush_count = 0;
#if SERIAL_DEBUG
	Serial.println(" done");
#endif
}

