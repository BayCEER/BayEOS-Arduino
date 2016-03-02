#include "BaySoftwareSerial.h"
BaySoftwareSerial::BaySoftwareSerial(uint8_t rxPin, uint8_t txPin,int timeout):SoftwareSerial(rxPin,txPin){
	BaySerialInterface::_timeout=timeout;
}

