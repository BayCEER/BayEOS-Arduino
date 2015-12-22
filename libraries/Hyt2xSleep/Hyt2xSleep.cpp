#include <inttypes.h>
#include <Wire.h>
#include <Sleep.h>
#include "Arduino.h"
#include "Hyt2xSleep.h"

void HYT2xClass::triggerMeasurement(){
	Wire.beginTransmission(HYT221_ADDR);
	Wire.write(0);
	Wire.available();
	int Ack = Wire.read(); // receive a byte

}

uint8_t HYT2xClass::readMeasurement(){
    while(! Wire.requestFrom(HYT221_ADDR, 4)) {
    	Sleep.sleep(TIMER2_ON | TWI_ON,SLEEP_MODE_PWR_SAVE);; //Sleep here
    }
    while( Wire.available() < 4){
    	;
    }


	if (Wire.available() > 3) {
		*(((uint8_t*) &valuecap) + 1) = (Wire.read() & 0x3f); //Mask Status Bits
		*((uint8_t*) &valuecap) = Wire.read();
		*(((uint8_t*) &valuetemp) + 1) = Wire.read();
		*((uint8_t*) &valuetemp) = Wire.read();
		valuetemp = valuetemp >> 2;
	    Wire.endTransmission();
		return 0;
	}
    Wire.endTransmission();
	return 1;

}

uint8_t HYT2xClass::measure(void){
	triggerMeasurement();
	return readMeasurement();
}

/**********************************************************
 * GetHumidity
 *  Gets the current humidity from the sensor.
 *
 * @return float - The relative humidity in %RH
 **********************************************************/
float HYT2xClass::GetHumidity(void)
{
	return ((float) valuecap * 100 / (1 << 14));
}

/**********************************************************
 * GetTemperature
 *  Gets the current temperature from the sensor.
 *
 * @return float - The temperature in Deg C
 **********************************************************/
float HYT2xClass::GetTemperature(void)
{
	return ((float) valuetemp * 165 / (1 << 14)) - 40;
}

HYT2xClass HYT2x;
