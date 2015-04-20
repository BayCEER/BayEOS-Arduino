/**
*
* Arduino Vaisala PWD11
* Oliver Archner (oliver.archner@uni-bayreuth.de
* 
* Class to read values of Vaisala Present Weather Detector PWD11
* The sensor pushes messages at regular intervals over a RS232 line. 
*
**/


#ifndef PWD11_H
#define PWD11_H
#include <SoftwareSerial.h> 

class PWD11 : public SoftwareSerial{

private:
	float getFloat(char* buffer,int startIndex, int length);
public:
		
	// Creates a new class. Argument RX and TX pin number. 	
    PWD11(uint8_t,uint8_t);
	
	/*
		Opens a serial connection 
		with 9600 Baud, 7 data bits, 1 stop bit and even parity 
	*/
    void begin(); 
	
	/*
		Reads Message 2 with 10 Values
		Waits until a valid message was sent 
	*/
	float* readMessage2();
};



#endif 