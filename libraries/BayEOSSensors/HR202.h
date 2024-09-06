/*
 * Calibration curve for HR202 connected with a __100k__ preresistor
 * Calibration is temperature dependent. Therefor one has to give a
 * temperature value.
 */

#ifndef HR202_h
#define HR202_h
#include <Arduino.h>


class HR202 {
private:
	uint8_t _vcc_pin;
	uint8_t _gnd_pin;
	uint8_t _adc_pin;
public:
	HR202(uint8_t vcc_pin, uint8_t gnd_pin,uint8_t adc_pin);
	float getHumidity(float t);
};




#endif
