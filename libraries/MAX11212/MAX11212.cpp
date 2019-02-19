#include "MAX11212.h"
#include <Sleep.h>


MAX11212::MAX11212(uint8_t sck,uint8_t dout){
	_sck=sck;
	_dout=dout;
}

void MAX11212::begin(void){
	pinMode(_sck,OUTPUT);
	pinMode(_dout,INPUT_PULLUP);
	digitalWrite(_sck,HIGH);
}

long MAX11212::read(bool cal,int timeout){
	digitalWrite(_sck,LOW);
	while(digitalRead(_dout)){
		timeout--;
		if(! timeout) return 0x8000000;
		Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE); // sleep function called here
	}
	long v=0;
	for(uint8_t i=0;i<(24+cal);i++){
		digitalWrite(_sck,HIGH);
		delayMicroseconds(1);
		digitalWrite(_sck,LOW);
		if(i<18)
			v|=(long)digitalRead(_dout)<<(23-i);
	}
	digitalWrite(_sck,HIGH);
	if(v & 0x00800000) v|=0xff000000;
	v/=64;
	return v; //value between -2^17 and  2^17-1
}
