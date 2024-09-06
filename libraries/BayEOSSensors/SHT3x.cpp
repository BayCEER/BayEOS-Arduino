/***************************************************
 This is a library for the SHT31 Digital Humidity & Temp Sensor


 ****************************************************/

#include "SHT3x.h"

SHT3x::SHT3x(uint8_t dataPin, uint8_t clockPin, uint8_t address):SoftI2C(dataPin, clockPin)  {
	// Initialize private storage for library functions
	_i2caddr = address;
}

uint16_t SHT3x::readStatus(void) {
	writeCommand(SHT31_READSTATUS);
	startTransmission();
	write((_i2caddr << 1) + 1);
	stopTransmission();
	uint16_t stat = read(true);
	stat <<= 8;
	stat |= read(false);
	//Serial.println(stat, HEX);
	return stat;
}

void SHT3x::reset(void) {
	writeCommand(SHT31_SOFTRESET);
	delay(10);
}

void SHT3x::heater(boolean h) {
	if (h)
		writeCommand(SHT31_HEATEREN);
	else
		writeCommand(SHT31_HEATERDIS);
}

int8_t SHT3x::measureSleep(float* t, float* h, uint8_t timeoutcounter) {
	return measure(t,h,timeoutcounter,true);
}

int8_t SHT3x::measure(float* t, float* h, uint8_t timeoutcounter, bool sleep) {
	*t=NAN;
	*h=NAN;
	uint8_t readbuffer[6];
	int8_t error = 0;
	error = writeCommand(SHT31_MEAS_HIGHREP_STRETCH);

	if (!error) {
		startTransmission();
		error = write((_i2caddr << 1) + 1);
	}
	if (!error) {
		pinMode(_pinClock, INPUT_PULLUP);

		while (!digitalRead(_pinClock)) {
			if (sleep)
				Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
			else
				delay(1000 / 16);
			timeoutcounter--;
			if (!timeoutcounter)
				return (S_Err_TO);
		}
		for (uint8_t i = 0; i < 6; i++) {
			readbuffer[i] = read(i < 5);
			//  Serial.print("0x"); Serial.println(readbuffer[i], HEX);
		}
	}
	stopTransmission();
	if(error) return error;
	uint16_t ST, SRH;
	ST = readbuffer[0];
	ST <<= 8;
	ST |= readbuffer[1];

	if (readbuffer[2] != crc8(readbuffer, 2))
		return S_Err_CRC;

	SRH = readbuffer[3];
	SRH <<= 8;
	SRH |= readbuffer[4];

	if (readbuffer[5] != crc8(readbuffer + 3, 2))
		return S_Err_CRC;

	// Serial.print("ST = "); Serial.println(ST);
	double stemp = ST;
	stemp *= 175;
	stemp /= 0xffff;
	stemp = -45 + stemp;
	*t = stemp;

//  Serial.print("SRH = "); Serial.println(SRH);
	double shum = SRH;
	shum *= 100;
	shum /= 0xFFFF;

	*h = shum;

	return 0;
}

int8_t SHT3x::writeCommand(uint16_t cmd) {
	startTransmission();
	int8_t error = 0;
	;
	if (!error)
		error = write((_i2caddr << 1) + 0); // +0 = write bit.
	if (!error)
		error = write(cmd >> 8);
	if (!error)
		error = write(cmd & 0xFF);
	stopTransmission();
	return error;
}

uint8_t SHT3x::crc8(const uint8_t *data, int len) {
	/*
	 *
	 * CRC-8 formula from page 14 of SHT spec pdf
	 *
	 * Test data 0xBE, 0xEF should yield 0x92
	 *
	 * Initialization data 0xFF
	 * Polynomial 0x31 (x8 + x5 +x4 +1)
	 * Final XOR 0x00
	 */

	const uint8_t POLYNOMIAL(0x31);
	uint8_t crc(0xFF);

	for (int j = len; j; --j) {
		crc ^= *data++;

		for (int i = 8; i; --i) {
			crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
		}
	}
	return crc;
}



/*********************************************************************/
