
#include "DS18B20.h"

DS18B20::DS18B20(uint8_t pin,uint8_t channel_offset,uint8_t ds18b20_channels) : OneWire(pin){
	_channel_offset=channel_offset;
	_current_channel=0;
	_addr=(uint8_t*) malloc(8*ds18b20_channels);
	_ds18b20_channels=ds18b20_channels;
  for (byte ii=0; ii<_ds18b20_channels; ++ii)
    for (byte jj=0; jj<8; ++jj) _addr[ii+jj*8] = 0x0;
}


uint8_t DS18B20::getNextChannel(void){
	while(_current_channel<_ds18b20_channels){
		_current_channel++;
		if(_addr[(_current_channel-1-_channel_offset)*8]) return _current_channel;
	}
	_current_channel=0;
	return _current_channel;
}

void DS18B20::t_conversion(void){
	_current_channel=0;
	reset();
	write(0xCC,1); // skip rom
	write(0x44,1); // start conversion
}

const uint8_t* DS18B20::search(void){
    reset_search();
    while(OneWire::search(_new_addr)){
        if(OneWire::crc8(_new_addr, 7) !=_new_addr[7]) continue;
        if (_new_addr[0] != 0x28) continue;
        if(! getChannel(_new_addr)) return _new_addr;
    }
    return 0;
}

uint8_t DS18B20::checkSensors(void){
	uint8_t i,j;
	for(i=0;i<_ds18b20_channels;i++){
		if(_addr[i*8]){
			reset();
			select(&_addr[i*8]);
			write(0xBE);         // Read Scratchpad
			for (j = 0; j < 9; j++) {           // we need 9 uint8_ts
				_data[j] = read();
			}
			if(_data[0]==0xFF && _data[1]==0xFF
					&& _data[2]==0xFF && _data[3]==0xFF ) return (i+1+_channel_offset);
		}
	}
	return 0;
}


uint8_t DS18B20::addSensor(const uint8_t* new_addr,uint8_t channel){
	int addr_offset=(channel-1-_channel_offset)*8;
	for(uint8_t i=0;i<8;i++){
		_addr[addr_offset+i]=new_addr[i];
	}
	writeAddrToEEPROM(channel);
}

uint8_t DS18B20::readAddrFromEEPROM(uint8_t channel){
	int ee_offset=(channel-1)*8;
	int addr_offset=(channel-1-_channel_offset)*8;
	if(! EEPROM.read(ee_offset)) return 0;
	for(uint8_t i=0;i<8;i++){
		_addr[addr_offset+i]=EEPROM.read(ee_offset+i);
	}
	if(OneWire::crc8(&_addr[addr_offset], 7) !=_addr[addr_offset+7]){
		_addr[addr_offset]=0x0;
		writeAddrToEEPROM(channel);
		return 0;
	}
	return 1;
}

uint8_t DS18B20::setAllAddrFromEEPROM(){
	uint8_t sensors=0;
	for(uint8_t i=0;i<_ds18b20_channels;i++){
		sensors+=readAddrFromEEPROM(i+1+_channel_offset);
	}
	return sensors;
}

int DS18B20::setAllAddr(){
	uint8_t channel;
	int count=0;
	const uint8_t* new_addr;
    while(channel=checkSensors()){
     new_addr=getChannelAddress(channel);
     if(deleteChannel(new_addr)) count--;
    }
    while(new_addr=search()){
    	if(channel=getNextFreeChannel()){
    		addSensor(new_addr,channel);
    		count++;
    	} else break;
    }
    return count;
}

void DS18B20::writeAddrToEEPROM(uint8_t channel){ //Channel beginnt mit 1
	int ee_offset=(channel-1)*8;
	int addr_offset=(channel-1-_channel_offset)*8;
	for(uint8_t i=0;i<8;i++){
		EEPROM.write(ee_offset+i,_addr[addr_offset+i]);
	}
}


uint8_t DS18B20::readChannel(uint8_t channel, float* f,uint8_t tries){
	int addr_offset=(channel-1-_channel_offset)*8;
	if(! _addr[addr_offset]) return 0x1; // not set!
	uint8_t i;
	while(tries){
		tries--;
		reset();
		select(&_addr[addr_offset]);
		write(0xBE);         // Read Scratchpad
		for (i = 0; i < 9; i++) {           // we need 9 uint8_ts
			_data[i] = read();
		}
		if ( OneWire::crc8( _data, 8) != _data[8] && _data[7]!=0xFF){
			if(! tries) return 0x2; //not valid CRC
		} else tries=0;
	}
	if(_data[7]==0xFF) *((long unsigned *) f)=NAN;
	else  *f = *(int16_t *)&_data[0]*0.0625; //_data[0]_data[1] is a LE-INT16
	return 0x0; // ok!
}

uint8_t DS18B20::getNextFreeChannel(void){
	for(uint8_t i=0;i<_ds18b20_channels;i++){
		if(_addr[i*8]==0x0) return (i+1+_channel_offset);
	}
	return 0;
}

uint8_t DS18B20::deleteChannel(const uint8_t* addr){
	uint8_t channel=getChannel(addr);
	if(channel){
		_addr[(channel-1-_channel_offset)*8]=0x0;
		writeAddrToEEPROM(channel);
	}
	return channel;
}

const uint8_t* DS18B20::getChannelAddress(uint8_t channel) const{
	return &_addr[(channel-1-_channel_offset)*8];
}

uint8_t DS18B20::getChannel(const uint8_t* addr){
	for(uint8_t i=0;i<_ds18b20_channels;i++){
	    if(addr[0]==_addr[i*8] && addr[1]==_addr[i*8+1] &&
	      addr[2]==_addr[i*8+2] && addr[3]==_addr[i*8+3] &&
	      addr[4]==_addr[i*8+4] && addr[5]==_addr[i*8+5] &&
	      addr[6]==_addr[i*8+6] && addr[7]==_addr[i*8+7]) return (i+1+_channel_offset);
	  }
	  return 0;
}


const String DS18B20::addr2String(const uint8_t* addr){
	String s=String("0x");
	for(uint8_t i=0;i<8;i++){
		s+=String(addr[i],16);
	}
	return s;
}


uint8_t DS18B20::getNumberOfChannels(void){return (uint8_t) _ds18b20_channels;}
