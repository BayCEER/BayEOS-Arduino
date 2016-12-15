#include "BayEOSBufferRAM.h"

BayEOSBufferRAM::BayEOSBufferRAM(uint16_t max_length,uint16_t min_free):BayEOSBuffer(){
	while((_buffer=(uint8_t *)malloc(max_length))==NULL){
		max_length-=10;
	}
	free(_buffer);
	max_length-=min_free;
	_buffer=(uint8_t *)malloc(max_length);
	_ram_pos=_buffer;
	_max_length=max_length;
	reset();
}

BayEOSBufferRAM::BayEOSBufferRAM(uint8_t* buffer,unsigned long max_length):BayEOSBuffer(){
	_buffer=buffer;
	_ram_pos=_buffer;
	_max_length=max_length;
	reset();
}


uint8_t BayEOSBufferRAM::write(const uint8_t b){
	if((_ram_pos-_buffer)<_max_length){
		*_ram_pos=b;
		_ram_pos++;
		return 1;
	} else return 0;
}

uint8_t BayEOSBufferRAM::write(const uint8_t *b,uint8_t length){
	if((_ram_pos-_buffer)+length<=_max_length){
		memcpy(_ram_pos,b,length);
		_ram_pos+=length;
		return length;
	} return 0;
}

uint8_t BayEOSBufferRAM::seek(unsigned long pos){
	if(pos<_max_length){
		_ram_pos=_buffer+pos;
		return true;
	} else return false;
}

int BayEOSBufferRAM::read(void){
	return *_ram_pos++;
}

int BayEOSBufferRAM::read(uint8_t *dest,int length){
	memcpy(dest,_ram_pos,length);
	_ram_pos+=length;
	return length;
}

void BayEOSBufferRAM::flush(void){
}
