
#ifndef BayEOSLOGGER_H
#define BayEOSLOGGER_H

#define BayEOS_LOGGER_VERSION "1.7"

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

typedef void (*reset_callback_function)(void);

class BayEOSLogger
{
public:
  BayEOSLogger()
  {
    _client = NULL;
    _buffer = NULL;
    _rtc = NULL;
    _mode = 0;
  }
  void init(BayEOS &client, BayEOSBuffer &buffer, RTC &rtc, uint16_t min_sampling_int = 10, uint16_t bat_warning = 0);
  void restoreReadPointerFromEEPROM(void);
  void setChannelMap(char *map);
  void setUnitMap(char *map);
  void handleCommand1_5(void);
  void handleCommand(void);
  void logData(void);
  void sendBinaryDump(void);
  void liveData(uint16_t wait);
  void run(bool connected = true);
  void setClient(BayEOS &client);
  void setResetCallback(reset_callback_function callback);
  uint16_t _bat;
  uint8_t _logging_disabled;
  int secondsSinceLastCommunication(void);
  uint8_t _logged_flag; // this is set when data is written to buffer
  uint8_t _mode;
  unsigned long _last_communication;

private:
  unsigned long _next_log;
  unsigned long _long1, _long2, _long3; // used to store time and pos information

  char *_channel_map;
  char *_unit_map;
  uint16_t _min_sampling_int;
  uint16_t _sampling_int;
  uint16_t _bat_warning;
  int8_t _framesize;
  BayEOS *_client;
  RTC *_rtc;
  reset_callback_function _reset_callback;
  BayEOSBuffer *_buffer;
  // could be used to reset average calculations
};

#endif
