#define ENABLE_PIN 4


#include <MAX11212.h>

MAX11212 adc(A5,A4);

void initDendro(void){
    adc.begin();
	pinMode(ENABLE_PIN,OUTPUT);
	digitalWrite(ENABLE_PIN,HIGH);
}

float readDendro( uint16_t delaytime=2){
	digitalWrite(ENABLE_PIN,LOW);
	delay(delaytime);
	float v=1.0*adc.read()/131071;
	digitalWrite(ENABLE_PIN,HIGH);
	return v;
}
