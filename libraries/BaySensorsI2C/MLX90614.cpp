/*************************************************** 
 This is a library for the MLX90614 Temp Sensor

 Designed specifically to work with the MLX90614 sensors in the
 adafruit shop
 ----> https://www.adafruit.com/products/1748
 ----> https://www.adafruit.com/products/1749

 These sensors use I2C to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "MLX90614.h"

MLX90614::MLX90614(uint8_t i2caddr) {
	_addr = i2caddr;
}

boolean MLX90614::begin(void) {
	Wire.begin();

	/*
	 for (uint8_t i=0; i<0x20; i++) {
	 Serial.print(i); Serial.print(" = ");
	 Serial.println(read16(i), HEX);
	 }
	 */
	return true;
}

//////////////////////////////////////////////////////

double MLX90614::readObjectTempF(uint8_t tries) {
	return (readTemp(MLX90614_TOBJ1,tries) * 9 / 5) + 32;
}

double MLX90614::readAmbientTempF(uint8_t tries) {
	return (readTemp(MLX90614_TA,tries) * 9 / 5) + 32;
}

double MLX90614::readObjectTempC(uint8_t tries) {
	return readTemp(MLX90614_TOBJ1,tries);
}

double MLX90614::readAmbientTempC(uint8_t tries) {
	return readTemp(MLX90614_TA,tries);
}

float MLX90614::readTemp(uint8_t reg,uint8_t tries) {
	float temp;

	while (tries) {
		temp = read16(reg);
		if (temp) {
			temp *= .02;
			temp -= 273.15;
			return temp;

		}
		delay(100);
		tries--;
	}
	return NAN;
}

void MLX90614::enterSleepMode(void) {
	uint8_t crc = crc8(0, (_addr << 1));
	crc = crc8(crc, MLX90614_SLEEP_MODE);
	Wire.beginTransmission(_addr); // start transmission to device
	Wire.write(MLX90614_SLEEP_MODE);
	Wire.write(crc);
	Wire.endTransmission(); // end transmission

	Wire.end();
	// Set the SCL pin LOW, and SDA pin HIGH (should be pulled up)
	pinMode(SCL, OUTPUT);
	digitalWrite(SCL, LOW);
	pinMode(SDA, INPUT_PULLUP);
}

void MLX90614::exitSleepMode(int t_delay) {
	Wire.end();
	pinMode(SCL, INPUT_PULLUP); // SCL high
	pinMode(SDA, OUTPUT);
	digitalWrite(SDA, LOW); // SDA low
	delay(50); // delay at least 33ms
	pinMode(SDA, INPUT_PULLUP); // SDA high
	delay(t_delay);
	// PWM to SMBus mode:
	pinMode(SCL, OUTPUT);
	digitalWrite(SCL, LOW); // SCL low
	delay(10); // Delay at least 1.44ms
	pinMode(SCL, INPUT_PULLUP); // SCL high
	Wire.begin();
}
/*********************************************************************/

uint16_t MLX90614::read16(uint8_t a) {
	Wire.beginTransmission(_addr); // start transmission to device
	Wire.write(a); // sends register address to read from
	Wire.endTransmission(false); // end transmission
	Wire.requestFrom(_addr, (uint8_t) 3); // send data n-bytes read
	uint8_t lsb = Wire.read();
	uint8_t msb = Wire.read();
	uint8_t pec = Wire.read();
	uint8_t crc = crc8(0, (_addr << 1));
	crc = crc8(crc, a);
	crc = crc8(crc, (_addr << 1) + 1);
	crc = crc8(crc, lsb);
	crc = crc8(crc, msb);
	if (crc == pec) {
		return (msb << 8) | lsb;
	}
	//Serial.println("CRC failed");
	return 0;

}

uint8_t MLX90614::crc8(uint8_t inCrc, uint8_t inData) {
	uint8_t i;
	uint8_t data;
	data = inCrc ^ inData;
	for (i = 0; i < 8; i++) {
		if ((data & 0x80) != 0) {
			data <<= 1;
			data ^= 0x07;
		} else {
			data <<= 1;
		}
	}
	return data;
}

