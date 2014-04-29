/*
 * Arduino BayBluetooth
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
 *
 * Class for sending BayEOS-Frames over a bluetooth serial Line
 *
 * Frames send over serial:
 * [0xfe][length_of_data][API-ID][------------DATA-----------][CHECKSUM]
 *
 *
 * START_BYTE,ESCAPE,XON,XOFF,NEWLINE,CARRIDGE RETURN
 *  in [length,api-id,data,checksum] will get escaped
 * (see below)
 *
 *
 */

#ifndef BayBluetooth_h
#define BayBluetooth_h

#include <inttypes.h>
#include <BayEOS.h>
#include "../BaySerial/BaySerial.h"
#include <HardwareSerial.h>
#include <Arduino.h>
#include <EEPROM.h>


class BayBluetooth : public BaySerial {
public:
	/**
	 * Constructor
	 */
	BayBluetooth(HardwareSerial &serial=Serial);

	/**
	 * Init BT-Module as slave with baud and name
	 */
	void begin(long baud,const char* name);
	/**
	 * Init BT-Module as slave with baud and name read from EEPROM
	 */
	void begin(long baud,int eeprom_offset, uint8_t start_byte);
	/**
	 * make BT-Module inquirable
	 */
	void inquirable(void);


};


#endif
