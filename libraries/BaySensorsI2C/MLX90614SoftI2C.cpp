#include "MLX90614SoftI2C.h"

MLX90614SoftI2C::MLX90614SoftI2C(uint8_t dataPin, uint8_t clockPin, uint8_t i2caddr):SoftI2C(dataPin, clockPin) {
  _addr = i2caddr;
}


//////////////////////////////////////////////////////


double MLX90614SoftI2C::readObjectTempF(void) {
  return (readTemp(MLX90614_TOBJ1) * 9 / 5) + 32;
}


double MLX90614SoftI2C::readAmbientTempF(void) {
  return (readTemp(MLX90614_TA) * 9 / 5) + 32;
}

double MLX90614SoftI2C::readObjectTempC(void) {
  return readTemp(MLX90614_TOBJ1);
}


double MLX90614SoftI2C::readAmbientTempC(void) {
  return readTemp(MLX90614_TA);
}

float MLX90614SoftI2C::readTemp(uint8_t reg) {
  float temp;

  temp = read16(reg);
  temp *= .02;
  temp  -= 273.15;
  return temp;
}

void MLX90614SoftI2C::enterSleepMode(void){
	beginTransmission(_addr); // start transmission to device
	write(MLX90614_SLEEP_MODE);
//	Wire.write(0xF3);
	write(0xE8);
	stopTransmission(); // end transmission
}


void MLX90614SoftI2C::exitSleepMode(int t_delay){
	pinMode(_pinData,OUTPUT);
	digitalWrite(_pinData, LOW);
	delay(36);
	digitalWrite(_pinData, HIGH);
	pinMode(_pinData,INPUT_PULLUP);
	delay(t_delay);
}
/*********************************************************************/

uint16_t MLX90614SoftI2C::read16(uint8_t a) {
  uint16_t ret;

  beginTransmission(_addr); // start transmission to device
  write(a); // sends register address to read from
  stopTransmission(); // end transmission

  requestFrom(_addr);// send data n-bytes read
  ret = read(true); // receive DATA
  ret |= read(true) << 8; // receive DATA
  uint8_t pec = read();

  return ret;
}
