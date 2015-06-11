
#ifndef BayEOSLOGGER_H
#define BayEOSLOGGER_H

#include <BayEOS.h>
#include <BayEOSBuffer.h>
#include <EEPROM.h>

#define EEPROM_NAME_STARTBYTE 0x7e
#define EEPROM_NAME_OFFSET 512
#define EEPROM_READ_POS_OFFSET 508
#define EEPROM_SAMPLING_INT_OFFSET 506

#define LOGGER_MODE_LIVE 0x1
#define LOGGER_MODE_DATA 0x2
#define LOGGER_MODE_DUMP 0x3


class BayEOSLogger
{
public:
  BayEOSLogger(){
	  _client=NULL;
	  _buffer=NULL;
	  _rtc=NULL;
	  _mode=0;
  }
  void init(BayEOS& client,BayEOSBuffer& buffer,RTC& rtc,uint16_t min_sampling_int=10);

  void handleCommand(void);
  void logData(void);
  void sendData(void);
  void sendBinaryDump(void);
  void liveData(uint16_t wait);
  void run();

  unsigned long _last_measurement;
  unsigned long _long1, _long2, _long3; //used to store time and pos information

  uint16_t _min_sampling_int;
  uint16_t _sampling_int;
  uint8_t _mode;
  BayEOS* _client;
  RTC* _rtc;
  BayEOSBuffer* _buffer;
  uint8_t _logging_disabled;
  uint8_t _logged_flag; //this is set when data is written to buffer
  //could be used to reset average calculations
  uint8_t _bufferwrap; //used to indicate that there is a bufferwrap in binary dump
};


#endif
