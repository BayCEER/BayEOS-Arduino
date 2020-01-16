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
	_flash->eraseSector(0); //only erase Sector 0
	_flash->eraseSector(_max_length); //erase pointer region
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
		_flash->eraseSector(end_pos);

		_flush_count=_flush_skip;
#if SERIAL_DEBUG
		Serial.println("Erase sector");
#endif
		seek(start_pos);
	}

}

uint8_t BayEOSBufferSPIFlash::write(const uint8_t b) {
	if (_pos < _max_length) {
		checkEraseSector(_pos, _pos + 1);
		_flash->writeByte(_pos, b, false);
		//_pos++;
		return 1;

	}
	return 0;
}
uint8_t BayEOSBufferSPIFlash::write(const uint8_t *b, uint8_t length) {
	if (_pos + length <= _max_length) {
		checkEraseSector(_pos, _pos + length);
		_flash->writeByteArray(_pos, b, length, false);
		//_pos += length;
		return length;
	}
	return 0;
}

uint8_t BayEOSBufferSPIFlash::seek(unsigned long pos) {
	if (pos < _max_length) {
		//_pos = pos;
		return true;
	} else
		return false;
}

int BayEOSBufferSPIFlash::read(void) {
	return _flash->readByte(_pos);
}

int BayEOSBufferSPIFlash::read(uint8_t *dest, int length) {
	_flash->readByteArray(_pos, dest, length);
	//_pos += length;
	return length;
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
		_flash->eraseSector(_max_length);
#if SERIAL_DEBUG
	Serial.print("Flush");
#endif
	_temp = 0x0f0f0f0fL;
	uint8_t *p;
	p = (uint8_t*) &_temp;
	_flash->writeByteArray(_max_length + 16 * _flush_offset, p, 4, false);

	p = (uint8_t*) &_read_pos;
	_flash->writeByteArray(_max_length + 16 * _flush_offset + 4, p, 4, false);
	p = (uint8_t*) &_write_pos;
	_flash->writeByteArray(_max_length + 16 * _flush_offset + 8, p, 4, false);
	p = (uint8_t*) &_end;
	_flash->writeByteArray(_max_length + 16 * _flush_offset + 12, p, 4, false);
	_flush_offset++;
	_flush_count = 0;
#if SERIAL_DEBUG
	Serial.println(" done");
#endif
}

