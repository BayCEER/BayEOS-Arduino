/**
 * Arduino DS18B20 Class extends OneWire Class
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
 *
 * Allows handling of DS18B20 temperature sensors
 * Class may take up to "ds18b20_channels" sensors on one bus
 * the sensors are registered in "channels"
 *
 */

#ifndef ds18b20_h
#define ds18b20_h
#define NAN 0x7fffffff
#ifndef DS18B20_CHANNELS
#define DS18B20_CHANNELS 16
#endif

#include <OneWire.h>
#include <inttypes.h>
#include <EEPROM.h>
#include <WString.h>



class DS18B20 : protected OneWire{
public:
	/**
	 * Constructor
	 */
	DS18B20(uint8_t pin,uint8_t channel_offset=0,uint8_t ds18b20_channels=DS18B20_CHANNELS);


	/**
	 * Destructor
	 */
	~DS18B20() { free(_addr); }



	/**
	 * starts temperature conversion
	 */
	void t_conversion(void);

	 /**
	  * search for unknown Sensors
	  * returns pointer to new address
	  */
	const uint8_t* search(void);

	 /**
	  * checks all known addresses
	  * returns channel of first non responding device
	  */
	uint8_t checkSensors(void);

	/**
	 * Can be used to iterate through all aktive channels
	 * returns 0 when no channel is left
	 */
	uint8_t getNextChannel(void);

	/**
	 * adds sensor with given address and channel number
	 */
	uint8_t addSensor(const uint8_t* new_addr,uint8_t channel);

	/**
	 * returns number of first free channel.
	 * returns 0 when no free channel is left
	 */
	uint8_t getNextFreeChannel(void);

	/**
	 * returns channel number of given address
	 * returns 0 when address is not found
	 */
	uint8_t getChannel(const uint8_t* addr);

	/**
	 * returns a pointer to the address of a given channel number
	 */
	const uint8_t* getChannelAddress(uint8_t channel) const;

	/**
	 * deletes given sensor address and frees channel
	 * returns freed channel number or zero on failure
	 */
	uint8_t deleteChannel(const uint8_t* addr);

	/**
	 * reads channel and puts the result in f
	 * return 0x0 on success
	 * 0x1 when there is no address for the channel
	 * 0x2 when there is data but not valid CRC8
	 */
	uint8_t readChannel(uint8_t channel,float* f,uint8_t tries=1);

	/**
	 * Reads address from EEPROM
	 * returns 1 for a valid address
	 * returns 0 when there is no address in EEPROM for the given channel
	 */
	uint8_t readAddrFromEEPROM(uint8_t channel);

	/**
	 * Reads all addresses from EEPROM
	 * returns the number of addresses read from EEPROM
	 * typically used at startup of arduino (e.g. after running out of battery)
	 */
	uint8_t setAllAddrFromEEPROM(void);

	/**
	 * Performs search and registration of all Sensors present at
	 * OneWire-bus
	 * returns number of sensors added or removed
	 */
	int setAllAddr(void);
  
	/**
	 * stores the address of given channel to EEPROM
	 */
	void writeAddrToEEPROM(uint8_t channel);

	/**
	 * Utility function for creating messages
	 */
	const String addr2String(const uint8_t* addr);


	/**
	 * returns the number of allocated channels
	 */
	uint8_t getNumberOfChannels(void);


private:
	uint8_t _channel_offset;
	uint8_t _ds18b20_channels;
	uint8_t* _addr;
	uint8_t _new_addr[8];
	uint8_t _data[12];
	uint8_t _current_channel;

};



#endif
