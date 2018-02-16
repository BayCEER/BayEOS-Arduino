#include "HR202.h"
#include <math.h>

HR202::HR202(uint8_t vcc_pin, uint8_t gnd_pin, uint8_t adc_pin) {
	_vcc_pin = vcc_pin;
	_gnd_pin = gnd_pin;
	_adc_pin = adc_pin;
}

float HR202::getHumidity(float t) {
	float a, b, c;
	float tt = t * t;
	a = 112.016090 - 0.360150168 * t + 1.156667e-03 * tt;
	b = -12.725041 - 0.066866381 * t - 1.365699e-04 * tt;
	c = 0.373017 - 0.006363128 * t + 5.289157e-05 * tt;
	float logR;
	analogReference (DEFAULT);
	pinMode(_vcc_pin, OUTPUT);
	digitalWrite(_vcc_pin, HIGH);
	if (_gnd_pin < 255) {
		pinMode(_gnd_pin, OUTPUT);
		digitalWrite(_gnd_pin, LOW);
	}
	int h = analogRead(_adc_pin);
	digitalWrite(_vcc_pin, LOW);
	if (_gnd_pin < 255) {
		digitalWrite(_gnd_pin, HIGH); //reverse current
		analogRead(_adc_pin);
		digitalWrite(_gnd_pin, LOW);
		pinMode(_gnd_pin,INPUT);
	}
	pinMode(_vcc_pin,INPUT);
	logR = log((float) 100 / ((float) 1023 / ((float) h) - 1));
	return a + b * logR + c * logR * logR;

}
