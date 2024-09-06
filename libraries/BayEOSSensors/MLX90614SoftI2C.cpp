#include "MLX90614SoftI2C.h"

MLX90614SoftI2C::MLX90614SoftI2C(uint8_t dataPin, uint8_t clockPin,
		uint8_t i2caddr) :
		SoftI2C(dataPin, clockPin) {
	_addr = i2caddr;
}

//////////////////////////////////////////////////////

double MLX90614SoftI2C::readObjectTempF(uint8_t tries) {
	return (readTemp(MLX90614_TOBJ1,tries) * 9 / 5) + 32;
}

double MLX90614SoftI2C::readAmbientTempF(uint8_t tries) {
	return (readTemp(MLX90614_TA,tries) * 9 / 5) + 32;
}

double MLX90614SoftI2C::readObjectTempC(uint8_t tries) {
	return readTemp(MLX90614_TOBJ1,tries);
}

double MLX90614SoftI2C::readAmbientTempC(uint8_t tries) {
	return readTemp(MLX90614_TA,tries);
}

float MLX90614SoftI2C::readTemp(uint8_t reg, uint8_t tries) {
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

void MLX90614SoftI2C::enterSleepMode(void) {
	// Calculate a crc8 value.
	// Bits sent: _deviceAddress (shifted left 1) + 0xFF
	uint8_t crc = crc8(0, (_addr << 1));
	crc = crc8(crc, MLX90614_SLEEP_MODE);

	// Manually send the sleep command:
	beginTransmission(_addr);
	write(MLX90614_SLEEP_MODE);
	write(crc);
	stopTransmission();

	// Set the SCL pin LOW, and SDA pin HIGH (should be pulled up)
	pinMode(_pinClock, OUTPUT);
	digitalWrite(_pinClock, LOW);
	pinMode(_pinData, INPUT_PULLUP);
}

void MLX90614SoftI2C::exitSleepMode(int t_delay) {
	// Wake operation from datasheet
	pinMode(_pinClock, INPUT_PULLUP); // SCL high
	pinMode(_pinData, OUTPUT);
	digitalWrite(_pinData, LOW); // SDA low
	delay(50); // delay at least 33ms
	pinMode(_pinData, INPUT_PULLUP); // SDA high
	delay(t_delay);
	// PWM to SMBus mode:
	pinMode(_pinClock, OUTPUT);
	digitalWrite(_pinClock, LOW); // SCL low
	delay(10); // Delay at least 1.44ms
	pinMode(_pinClock, INPUT_PULLUP); // SCL high
}
/*********************************************************************/

uint16_t MLX90614SoftI2C::read16(uint8_t a) {
	int8_t error = 0;

	startTransmission();
	error = write((_addr << 1)); //Write bit
	if (!error)
		error = write(a); // sends register address to read from
	//DO NOT CALL stopTransmission() here!

	if (error) {
		stopTransmission();
		return 0;
	}

	startTransmission();
	error = write((_addr << 1) + 1); //Read bit
	if (error) {
		stopTransmission();
		return 0;
	}
	uint8_t lsb = read(true); // receive DATA
	uint8_t msb = read(true); // receive DATA
	uint8_t pec = read();
	stopTransmission();
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

uint8_t MLX90614SoftI2C::crc8(uint8_t inCrc, uint8_t inData) {
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
