/***************************************************
 This is a library for the SHT31 Digital Humidity & Temp Sensor


 ****************************************************/

#include "SHT3x.h"

SHT3x::SHT3x(uint8_t dataPin, uint8_t clockPin, uint8_t address) {
	// Initialize private storage for library functions
	_pinData = dataPin;
	_pinClock = clockPin;
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

// Write byte to sensor and check for acknowledge
int8_t SHT3x::write(uint8_t value) {
	uint8_t mask;
	int8_t i;
	int8_t error = 0;

	pinMode(_pinClock, OUTPUT);         // Set clock line to output mode
	pinMode(_pinData, OUTPUT);          // Set data line to output mode

	mask = 0x80;                        // Bit mask to transmit MSB first
	for (i = 8; i > 0; i--) {
		digitalWrite(_pinData, value & mask);
		PULSE_SHORT;
		digitalWrite(_pinClock, HIGH);    // Generate clock pulse
		PULSE_LONG;
		digitalWrite(_pinClock, LOW);
		PULSE_SHORT;
		mask >>= 1;                       // Shift mask for next data bit
	}

	pinMode(_pinData, INPUT_PULLUP); // Return data line to input mode pullup resistor 20kohm
	PULSE_SHORT;

	digitalWrite(_pinClock, HIGH);      // Clock #9 for ACK
	PULSE_LONG;
	if (digitalRead(_pinData))          // Verify ACK ('0') received from sensor
		error = S_Err_NoACK;
	PULSE_SHORT;
	digitalWrite(_pinClock, LOW);       // Finish with clock in low state
	PULSE_SHORT;

	return error;
}

// Read byte from sensor and send acknowledge if "ack" is true
uint8_t SHT3x::read(bool ack) {
	int8_t i;
	uint8_t result = 0;

	pinMode(_pinClock, OUTPUT);         // Set clock line to output mode

	for (i = 8; i > 0; i--) {
		result <<= 1;                     // Shift received bits towards MSB
		digitalWrite(_pinClock, HIGH);    // Generate clock pulse
		PULSE_SHORT;
		result |= digitalRead(_pinData);  // Merge next bit into LSB position
		digitalWrite(_pinClock, LOW);
		PULSE_SHORT;
	}
	pinMode(_pinData, OUTPUT);
	digitalWrite(_pinData, !ack);       // Assert ACK ('0') if ack == 1
	PULSE_SHORT;
	digitalWrite(_pinClock, HIGH);      // Clock #9 for ACK / noACK
	PULSE_LONG;
	digitalWrite(_pinClock, LOW);       // Finish with clock in low state
	PULSE_SHORT;

	pinMode(_pinData, INPUT_PULLUP); // Return data line to input mode pullup resistor 20kohm
	PULSE_SHORT;

	return result;
}

void SHT3x::startTransmission(void) {
	digitalWrite(_pinData, HIGH);   // Set data register high before turning on
	pinMode(_pinData, OUTPUT);      // output driver (avoid possible low pulse)
	digitalWrite(_pinClock, HIGH);  // Set clk register high before turning on
	pinMode(_pinClock, OUTPUT);     // output driver (avoid possible low pulse)

	PULSE_SHORT;                    // Start transmission sequence
	digitalWrite(_pinData, LOW);
	PULSE_SHORT;
	digitalWrite(_pinClock, LOW);
	PULSE_SHORT;

}

void SHT3x::stopTransmission(void) {
	digitalWrite(_pinData, LOW);      // Set data register low before turning on
	pinMode(_pinData, OUTPUT);      // output driver (avoid possible high pulse)
	digitalWrite(_pinClock, LOW);      // Set clk register low before turning on
	pinMode(_pinClock, OUTPUT);     // output driver (avoid possible high pulse)

	PULSE_SHORT;                        // Stop transmission sequence
	digitalWrite(_pinClock, HIGH);
	PULSE_SHORT;
	digitalWrite(_pinData, HIGH);
	PULSE_SHORT;

	pinMode(_pinData, INPUT_PULLUP); // Return clock line to input mode pullup resistor 20kohm
	pinMode(_pinClock, INPUT_PULLUP); // Return clock line to input mode pullup resistor 20kohm
	PULSE_SHORT;
}

/*********************************************************************/
