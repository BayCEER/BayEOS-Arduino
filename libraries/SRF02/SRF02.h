#ifndef SRF02_H_
#define SRF02_H_

#include <Wire.h>
#include <Arduino.h>


/** Measurement modes */
const int SRF02_INCHES = 0x50;
const int SRF02_CENTIMETERS = 0x51;
const int SRF02_MICROSECONDS = 0x52;


class SRF02
{

	private:	
	void sendCommand (int deviceId, int command);
	void setRegister(int deviceId, int thisRegister);
	int readData(int deviceId, int numBytes);	

	public:
	/**
	 * Initialize SRF02. 
	 *
	 * @param deviceId The I2C device id
	 * @param mode Wether sensor values are read as inches, centimeters or miliseconds
	 */
	SRF02(int deviceId, int mode);
	/**
	*
	* * Initialize SRF02 with default values 
	*/
	SRF02();
	/**
	 * Get the sensor distance value.
	 * 
	 * @return The distance value
	 */
	int getDistance();			
	/** I2C device id */
	int _deviceId;
	/** Measurement mode */
	int _mode;	
	

	
};

#endif /*SRF02_H_*/
