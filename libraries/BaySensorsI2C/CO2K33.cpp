#include "CO2K33.h"

CO2K33::CO2K33() {
}

void CO2K33::wakeUp(void) {
	// This command serves as a wakeup to the CO2 sensor, for K33?ELG/BLG Sensors Only

	// You'll have the look up the registers for your specific device, but the idea here is simple:
	// 1. Disabled the I2C engine on the AVR
	// 2. Set the Data Direction register to output on the SDA line
	// 3. Toggle the line low for ~1ms to wake the micro up. Enable I2C Engine
	// 4. Wake a millisecond.

	TWCR &= ~(1 << 2); // Disable I2C Engine
	pinMode(SDA, OUTPUT);
	digitalWrite(SDA, LOW);
	delay(1);
	digitalWrite(SDA, HIGH);
	TWCR |= (1 << 2); // I2C is now enabled
	delay(1);
}

float CO2K33::readCO2(void) {
	return readRAM(0x08);
}
float CO2K33::readTemperature(void) {
	return readRAM(0x12) / 100;
}
float CO2K33::readHumidity(void) {
	return readRAM(0x14) / 100;
}

float CO2K33::readABC(void) {
	return readEEPROM(0x40) / 24;
}


void CO2K33::begin(void) {
	Wire.begin();
}

uint8_t CO2K33::writeRAM(uint8_t memaddr, uint8_t* command, uint8_t length) {
	wakeUp();
	Wire.beginTransmission(CO2K33_ADDRESS);
	Wire.write(0x10 | length);
	Wire.write(0x00);
	Wire.write(memaddr);
	memaddr += (0x10 | length); //checksum!
	while (length > 0) {
		Wire.write(*command);
		memaddr += *command; //checksum!
		command++;
		length--;
	}
	Wire.write(memaddr);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(CO2K33_ADDRESS, 2);
	i = 0;
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
	if (buffer[0] != buffer[1])
		return 2;
	if (buffer[0] == 0x11)
		return 0;
	if (buffer[0] == 0x10)
		return 1;
	return 3;
}

uint8_t CO2K33::zeroCalibration() {
	uint8_t c[] = { 0x7c, 0x07 };
	return writeRAM(0x42, c, 2);
}

uint8_t CO2K33::backgroundCalibration() {
	uint8_t c[] = { 0x7c, 0x06 };
	return writeRAM(0x42, c, 2);
}

uint8_t CO2K33::triggerMeasurement() {
	uint8_t c[] = { 0x35 };
	writeRAM(0x60, c, 1);
}

uint8_t CO2K33::disableABC() {
	uint8_t c[] = { 0x00, 0x00 };
	return writeEEPROM(0x40, c, 2);
}

float CO2K33::readRAM(uint8_t memaddr) {
	uint8_t count = 0;
	count++;
	wakeUp();
	Wire.beginTransmission(CO2K33_ADDRESS);
	Wire.write(0x22);
	Wire.write(0x00);
	Wire.write(memaddr);
	memaddr += 0x22; //checksum!
	Wire.write(memaddr);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(CO2K33_ADDRESS, 4);
	i = 0;
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
	if ((buffer[0] + buffer[1] + buffer[2]) != buffer[3])
		return NAN;

	value = 0;
	value |= buffer[1] & 0xff;
	value = value << 8;
	value |= buffer[2] & 0xff;

	return value;
}

float CO2K33::readEEPROM(uint8_t memaddr) {
	uint8_t count = 0;
	count++;
	wakeUp();
	Wire.beginTransmission(CO2K33_ADDRESS);
	Wire.write(0x42);
	Wire.write(0x00);
	Wire.write(memaddr);
	memaddr += 0x42; //checksum!
	Wire.write(memaddr);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(CO2K33_ADDRESS, 4);
	i = 0;
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
	if ((buffer[0] + buffer[1] + buffer[2]) != buffer[3])
		return NAN;

	value = 0;
	value |= buffer[1] & 0xff;
	value = value << 8;
	value |= buffer[2] & 0xff;

	return value;
}

uint8_t CO2K33::writeEEPROM(uint8_t memaddr, uint8_t* command, uint8_t length) {
	wakeUp();
	Wire.beginTransmission(CO2K33_ADDRESS);
	Wire.write(0x30 | length);
	Wire.write(0x00);
	Wire.write(memaddr);
	memaddr += (0x30 | length); //checksum!
	while (length > 0) {
		Wire.write(*command);
		memaddr += *command; //checksum!
		command++;
		length--;
	}
	Wire.write(memaddr);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(CO2K33_ADDRESS, 2);
	i = 0;
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
	if (buffer[0] != buffer[1])
		return 2;
	if (buffer[0] == 0x31)
		return 0;
	if (buffer[0] == 0x30)
		return 1;
	return 3;
}

