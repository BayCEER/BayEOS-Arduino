#include "BaySoftwareSerial.h"
BaySoftwareSerial::BaySoftwareSerial(uint8_t rxPin, uint8_t txPin,int timeout):SoftwareSerial(rxPin,txPin){
	BaySerialInterface::_timeout=timeout;
}

int BaySoftwareSerial::available(void){
	return SoftwareSerial::available();
}
int BaySoftwareSerial::i_available(void){
	return SoftwareSerial::available();
}
void BaySoftwareSerial::begin(long baud){
	SoftwareSerial::begin(baud);
}
void BaySoftwareSerial::flush(void){
	SoftwareSerial::flush();
}
void BaySoftwareSerial::end(void){
	SoftwareSerial::end();
}
int BaySoftwareSerial::read(void){
	return SoftwareSerial::read();
}
size_t BaySoftwareSerial::write(uint8_t c){
	return SoftwareSerial::write(c);
}
