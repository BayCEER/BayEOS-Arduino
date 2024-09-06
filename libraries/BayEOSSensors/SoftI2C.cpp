#include "SoftI2C.h"
SoftI2C::SoftI2C(uint8_t dataPin, uint8_t clockPin) {
	_pinClock = clockPin;
	_pinData = dataPin;
}
// Write byte to sensor and check for acknowledge
int8_t SoftI2C::write(uint8_t value) {
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
	if (digitalRead(_pinData)) {        // Verify ACK ('0') received from sensor
		error = S_Err_NoACK;
	}
//	PULSE_SHORT;
	digitalWrite(_pinClock, LOW);       // Finish with clock in low state
	PULSE_SHORT;

	return error;
}

// Read byte from sensor and send acknowledge if "ack" is true
uint8_t SoftI2C::read(bool ack) {
	int8_t i;
	uint8_t result = 0;
	pinMode(_pinClock, OUTPUT);
	pinMode(_pinData, INPUT_PULLUP);
	for (i = 8; i > 0; i--) {
		PULSE_SHORT;
		result <<= 1;                     // Shift received bits towards MSB
		digitalWrite(_pinClock, HIGH);
		PULSE_LONG;
		result |= digitalRead(_pinData);  // Merge next bit into LSB position
		digitalWrite(_pinClock, LOW);
		PULSE_SHORT;
	}
	pinMode(_pinData, OUTPUT);
	digitalWrite(_pinData, !ack);       // Assert ACK ('0') if ack == 1
	digitalWrite(_pinClock, HIGH);
	PULSE_LONG;
	digitalWrite(_pinClock, LOW);       // Finish with clock in low state
//	PULSE_SHORT;

	pinMode(_pinData, INPUT_PULLUP); // Return data line to input mode pullup resistor 20kohm
	PULSE_SHORT;

	return result;
}

void SoftI2C::startTransmission(void) {
	digitalWrite(_pinData, HIGH);   // Set data register high before turning on
	pinMode(_pinData, OUTPUT);      // output driver (avoid possible low pulse)
	PULSE_SHORT;
	digitalWrite(_pinClock, HIGH);  // Set clk register high before turning on
	pinMode(_pinClock, OUTPUT);     // output driver (avoid possible low pulse)

	PULSE_SHORT;                    // Start transmission sequence
	digitalWrite(_pinData, LOW);
	PULSE_SHORT;
	digitalWrite(_pinClock, LOW);
	PULSE_SHORT;

}

uint8_t SoftI2C::beginTransmission(uint8_t addr) {
	startTransmission();
	return write((addr << 1) + 0); // +0 = write bit.
}
uint8_t SoftI2C::requestFrom(uint8_t addr) {
	startTransmission();
	return write((addr << 1) + 1); // +1 = read bit.
	stopTransmission();
}

void SoftI2C::stopTransmission(void) {
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

