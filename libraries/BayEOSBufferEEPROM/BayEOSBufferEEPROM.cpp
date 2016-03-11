#include "BayEOSBufferEEPROM.h"
#define SERIAL_DEBUG 0
BayEOSBufferEEPROM::BayEOSBufferEEPROM():BayEOSBuffer(){}

void BayEOSBufferEEPROM::init(uint8_t i2c_address, unsigned long max_length, uint16_t pointer_offset){
	_max_length=max_length-pointer_offset;
	_eeprom=I2C_eeprom(i2c_address);
	_flush=(pointer_offset>0);
	if(_flush){
	//Restore pointers
		uint8_t *p;
		p=(uint8_t*)&_pos;
		_eeprom.readBlock(_max_length,p,4);
		if(_pos<_max_length) _read_pos=_pos;
		_eeprom.readBlock(_max_length+4,p,4);
		if(_pos<_max_length) _write_pos=_pos;
		_eeprom.readBlock(_max_length+8,p,4);
		if(_pos<_max_length) _end=_pos;
	}
}


uint8_t BayEOSBufferEEPROM::write(const uint8_t b){
	if(_pos<_max_length){
		_eeprom.writeByte(_pos,b);
		_pos++;
		return 1;
	} else return 0;
}

uint8_t BayEOSBufferEEPROM::write(const uint8_t *b,uint8_t length){
	if(_pos+length<_max_length){
		_eeprom.writeBlock(_pos,b,length);
		_pos+=length;
		return length;
	} return 0;
}

uint8_t BayEOSBufferEEPROM::seek(unsigned long pos){
	if(pos<_max_length){
		_pos=pos;
		return true;
	} else return false;
}

int BayEOSBufferEEPROM::read(void){
	return _eeprom.readByte(_pos++);
}

int BayEOSBufferEEPROM::read(uint8_t *dest,int length){
	_eeprom.readBlock(_pos,dest,length);
	_pos+=length;
	return length;
}

void BayEOSBufferEEPROM::flush(void){
	if(! _flush) return;
	//save the poiters to the last address region of the eeprom
	uint8_t *p;
	p=(uint8_t*)&_read_pos;
	_eeprom.writeBlock(_max_length,p,4);
	p=(uint8_t*)&_write_pos;
	_eeprom.writeBlock(_max_length+4,p,4);
	p=(uint8_t*)&_end;
	_eeprom.writeBlock(_max_length+8,p,4);
}

/*
 * MultiEEPROM
 *
 */
BayEOSBufferMultiEEPROM::BayEOSBufferMultiEEPROM():BayEOSBuffer(){}

void BayEOSBufferMultiEEPROM::init(uint8_t number,uint8_t* i2c_addresses, unsigned long ee_size, uint16_t pointer_offset){
	_max_length=(unsigned long)ee_size*number-pointer_offset;
	_ee_size=ee_size;
	_ee_mask=_ee_size-1;
	memcpy(_i2c_addresses,i2c_addresses,number);
	_flush=(pointer_offset>0);
	if (_flush) {
		uint8_t *p;
		p = (uint8_t*) &_pos;
		_eeprom = I2C_eeprom(getDevice(_max_length));
		_eeprom.readBlock(_max_length & _ee_mask, p, 4);
		if (_pos < _max_length)
			_read_pos = _pos;
		_eeprom.readBlock((_max_length + 4) & _ee_mask, p, 4);
		if (_pos < _max_length)
			_write_pos = _pos;
		_eeprom.readBlock((_max_length + 8) & _ee_mask, p, 4);
		if (_pos < _max_length)
			_end = _pos;
	}
}

uint8_t BayEOSBufferMultiEEPROM::getDevice(unsigned long pos){
	unsigned long device=pos & (~ _ee_mask);
	device/=_ee_size;
#if SERIAL_DEBUG
	Serial.print("getDevice:");
	Serial.println(_i2c_addresses[device]);
#endif
	return _i2c_addresses[device];
}

uint8_t BayEOSBufferMultiEEPROM::write(const uint8_t b){
	if(_pos<_max_length){
		_eeprom.setDeviceAddress(getDevice(_pos));
		_eeprom.writeByte(_pos & _ee_mask,b);
		_pos++;
		return 1;
	} else return 0;
}

uint8_t BayEOSBufferMultiEEPROM::write(const uint8_t *b,uint8_t length){
	if(getDevice(_pos)!=getDevice(_pos+length-1)){
		uint8_t first=_ee_size-(_pos & _ee_mask);
#if SERIAL_DEBUG
	Serial.print("write2:");
	Serial.print(_pos);
	Serial.print(" ");
	Serial.println(first);
#endif
		return (write(b,first)+write(b+first,length-first));
	}

	if(_pos+length<_max_length){
		_eeprom.setDeviceAddress(getDevice(_pos));
		_eeprom.writeBlock(_pos & _ee_mask,b,length);
		_pos+=length;
#if SERIAL_DEBUG
	Serial.print("write:");
	Serial.print(_pos);
	Serial.print(" ");
	Serial.println(length);
#endif
		return length;
	} return 0;
}

uint8_t BayEOSBufferMultiEEPROM::seek(unsigned long pos){
	if(pos<_max_length){
		_pos=pos;
		return true;
	} else return false;
}

int BayEOSBufferMultiEEPROM::read(void){
	_eeprom.setDeviceAddress(getDevice(_pos));
	return _eeprom.readByte(_pos++ & _ee_mask);
}

int BayEOSBufferMultiEEPROM::read(uint8_t *dest,int length){
	if(getDevice(_pos)!=getDevice(_pos+length-1)){
		uint8_t first=_ee_size-(_pos & _ee_mask);
		return (read(dest,first)+read(dest+first,length-first));
	}
	_eeprom.setDeviceAddress(getDevice(_pos));
	_eeprom.readBlock(_pos & _ee_mask,dest,length);
	_pos+=length;
	return length;
}

void BayEOSBufferMultiEEPROM::flush(void){
	if(! _flush) return;
	_eeprom.setDeviceAddress(getDevice(_max_length));
	uint8_t *p;
	p=(uint8_t*)&_read_pos;
	_eeprom.writeBlock(_max_length & _ee_mask,p,4);
	p=(uint8_t*)&_write_pos;
	_eeprom.writeBlock((_max_length+4) & _ee_mask,p,4);
	p=(uint8_t*)&_end;
	_eeprom.writeBlock((_max_length+8) & _ee_mask,p,4);
}

