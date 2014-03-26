#include "SRF02.h"

/** How long it takes for a SRF02 to perform a measurement (milli seconds) */
const int READ_DURATION = 70;
const int RESULT_REGISTER = 0x02;
const int DEVICE_ADDRESS = 0x70;

SRF02::SRF02(int deviceId, int mode)
{	this->_deviceId = deviceId;
	this->_mode = mode;	
	Wire.begin(); 
}

// Default Constructor 
SRF02::SRF02()
{	this->_deviceId = DEVICE_ADDRESS;
	this->_mode = SRF02_CENTIMETERS;	
	Wire.begin(); 
}

int SRF02::getDistance(){	 
	 sendCommand(_deviceId, _mode);
	 delay(READ_DURATION);
     setRegister(_deviceId, RESULT_REGISTER);   	  
	 return readData(_deviceId, 2);      
}


void SRF02::sendCommand (int deviceId, int command) {
  // start I2C transmission:
  Wire.beginTransmission(deviceId);
  // send command:
  Wire.write((byte)0x00);
  Wire.write((byte)command);
  // end I2C transmission:
  Wire.endTransmission();
}

void SRF02::setRegister(int deviceId, int thisRegister) {
  // start I2C transmission:
  Wire.beginTransmission(deviceId);
  // send address to read from:
  Wire.write(thisRegister);
  // end I2C transmission:
  Wire.endTransmission();
}


/*
  readData() returns a result from the SRF sensor
*/
int SRF02::readData(int deviceId, int numBytes) {
	int result = 0; // the result is two bytes long
	// send I2C request for data:
	Wire.requestFrom(deviceId, numBytes);
	// wait for two bytes to return:
	while (Wire.available() < 2 ) {
	// wait for result
	}
	// read the two bytes, and combine them into one int:
	result = Wire.read() * 256;
	result = result + Wire.read();
	return result;
}
