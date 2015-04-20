/**
*
* Arduino MB7364 
* Oliver Archner (oliver.archner@uni-bayreuth.de
* 
* Class to read range values of XRXL MaxSonar MB7364 sensors over RS232
* 
*
**/


#ifndef MB7364_H
#define MB7364_H
#include <SoftwareSerial.h> 

class MB7364 : public SoftwareSerial{
	
	
public:
	/*
		Creates a new class. First argument is the RX pin number. 
	*/
    MB7364(uint8_t, uint8_t);
	/*
		Opens a serial connection 
	*/
    void begin(); 
	/*
		Read range value of sensor 
	*/
	int range();
};

#endif 