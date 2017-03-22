//  This is a library for the K33 CO2 module connected to Arduino via I2C
#ifndef __RAD7SERIAL_H__
#define __RAD7SERIAL_H__

#include <BayEOSBuffer.h>
#include <BayEOS.h>

#define RAD7SERIAL_DEBUG 1

class RAD7Serial {
public:
	RAD7Serial(HardwareSerial  &serial,BayEOS &client, uint8_t maxPayload=100);
	int readData(void);
	uint8_t deleteData(void);
	int get_rnumber(void){
		return _rnumber;
	}
private:
	uint8_t readline(unsigned long timeout = 30000L, uint8_t l = 148);
	/*
	   Wait for the device to send a ">"
	   returns 0 on success
	   2: Timeout
	   1: Serial read problem
	*/
	uint8_t wait_for_ready(void);
	HardwareSerial* _serial; //Pointer to existing serial object!!
	BayEOS* _client; //Pointer to existing BayEOS Object
	char _buffer[150];
	uint8_t _saved[125];
	int _rnumber;
	uint8_t _maxChannel;
};


#endif
