#include "CO2K30.h"

CO2K30::CO2K30() {
}


float CO2K30::readCO2(void) {
	return readRAM(0x08);
}

float CO2K30::readABC(void) {
	return readEEPROM(0x40) / 24;
}


void CO2K30::begin(void) {
	Wire.begin();
}

uint8_t CO2K30::writeRAM(uint8_t memaddr, uint8_t* command, uint8_t length) {
	Wire.beginTransmission(CO2K30_ADDRESS);
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
	Wire.requestFrom(CO2K30_ADDRESS, 2);
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

uint8_t CO2K30::zeroCalibration() {
	uint8_t c[] = { 0x7c, 0x07 };
	return writeRAM(0x67, c, 2);
}

uint8_t CO2K30::backgroundCalibration() {
	uint8_t c[] = { 0x7c, 0x06 };
	return writeRAM(0x67, c, 2);
}


uint8_t CO2K30::disableABC() {
	uint8_t c[] = { 0x00, 0x00 };
	return writeEEPROM(0x40, c, 2);
}

float CO2K30::readRAM(uint8_t memaddr) {
	uint8_t count = 0;
	count++;
	Wire.beginTransmission(CO2K30_ADDRESS);
	Wire.write(0x22);
	Wire.write(0x00);
	Wire.write(memaddr);
	memaddr += 0x22; //checksum!
	Wire.write(memaddr);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(CO2K30_ADDRESS, 4);
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

float CO2K30::readEEPROM(uint8_t memaddr) {
	uint8_t count = 0;
	count++;
	Wire.beginTransmission(CO2K30_ADDRESS);
	Wire.write(0x42);
	Wire.write(0x00);
	Wire.write(memaddr);
	memaddr += 0x42; //checksum!
	Wire.write(memaddr);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(CO2K30_ADDRESS, 4);
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

uint8_t CO2K30::writeEEPROM(uint8_t memaddr, uint8_t* command, uint8_t length) {
	Wire.beginTransmission(CO2K30_ADDRESS);
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
	Wire.requestFrom(CO2K30_ADDRESS, 2);
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

