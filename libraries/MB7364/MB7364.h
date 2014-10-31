#ifndef MB7364_H
#define MB7364_H
#include <SoftwareSerial.h> 

class MB7364 : public SoftwareSerial{
public:
    MB7364(uint8_t, uint8_t);
    void begin(); 
	int range();
};

#endif 