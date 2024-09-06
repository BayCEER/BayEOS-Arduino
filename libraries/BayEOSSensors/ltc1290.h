/*#######################################################################################
AVR Ansteuerung eines LTC1290 mit Atmega32

*/
#include <inttypes.h>


class LTC1290{
public:
	LTC1290(uint8_t sclk,uint8_t din,uint8_t dout,uint8_t cs);
	void read(int *res,uint8_t rep=0);
	int read(uint8_t ch,uint8_t rep=0);
	void init(void);
private:
	uint8_t _sclk,_din,_dout,_cs;
};

