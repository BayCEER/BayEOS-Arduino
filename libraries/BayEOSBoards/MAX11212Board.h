#define ADR_A A2
#define ADR_B A1
#define ADR_C A0
#define INHIB A3

#ifndef MAX11212_REFERENCE
#define MAX11212_REFERENCE 3.3
#endif

#include <MAX11212.h>

MAX11212 adc(A5,A4);

void initMAX11212(void){
	pinMode(ADR_A,OUTPUT);
	pinMode(ADR_B,OUTPUT);
	pinMode(ADR_C,OUTPUT);
	pinMode(INHIB,OUTPUT);
	digitalWrite(INHIB,HIGH);
    adc.begin();
}


float readChannel(uint8_t nr, uint16_t delaytime=20){
	digitalWrite(ADR_A,nr&0x1);
	digitalWrite(ADR_B,nr&0x2);
	digitalWrite(ADR_C,nr&0x4);
	digitalWrite(INHIB,LOW);
	delay(delaytime);
	float v=MAX11212_REFERENCE*adc.read()/131071;
	digitalWrite(INHIB,HIGH);
	return v;
}


