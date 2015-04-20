//Modified RTC-Library
// S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
//
//
// A library for handling real-time clocks, dates, etc.
// 2010-02-04 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// $Id: RTClib.h 7763 2011-12-11 01:28:16Z jcw $

// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
#ifndef RTClib_H
#define RTClib_H

#include "../BayEOSBuffer/BayEOSBuffer.h"



// RTC based on the DS1307 chip connected via I2C and the Wire library
class RTC_DS1307 : public RTC {
public:
    void begin() {}
    void adjust(const DateTime& dt);
    DateTime now();

};

// RTC based on the PCF8563 chip connected via I2C and the Wire library
// contributed by @mariusster, see http://forum.jeelabs.net/comment/1902
class RTC_PCF8563 : public RTC {
public:
    void begin() {}
    void adjust(const DateTime& dt);
    DateTime now();

};

// RTC using the internal millis() clock, has to be initialized before use
// NOTE: this clock won't be correct once the millis() timer rolls over (>49d?)
class RTC_Millis : public RTC {
public:
    void begin() {}
    void adjust(const DateTime& dt);
    DateTime now();

protected:
    static long offset;
};
// RTC based on the DS3231
//based on DS3231 Class is by Seeed Technology Inc(http://www.seeedstudio.com)
class DS3231 : public RTC {
public:
    void begin();
    void adjust(const DateTime& dt);
    DateTime now();
    uint8_t readRegister(uint8_t regaddress);
    void writeRegister(uint8_t regaddress, uint8_t value);
    //Decides the /INT pin's output setting
    //periodicity can be any of following defines: EverySecond, EveryMinute, EveryHour
    void enableInterrupts(uint8_t periodicity);
    void enableInterrupts(uint8_t hh24, uint8_t mm,uint8_t ss);
    void disableInterrupts();
    void clearINTStatus();

    void convertTemperature();
    float getTemperature();
protected:
    uint8_t intType, intPeriodicity, intHH24, intMM;
};

// RTC RX8025 chip connected via I2C and uses the Wire library.
// Only 24 Hour time format is supported in this implementation
class R8025 : public RTC {
public:
    void begin(void);
    void adjust(const DateTime& dt);  //Changes the date-time
    DateTime now();            //Gets the current date-time

    //Decides the /INTA pin's output setting
    //periodicity cn be any of following defines: EverySecond, EveryMinute, EveryHour or EveryMonth
    void enableINTA_Interrupts(uint8_t periodicity);
    void enableINTA_Interrupts(uint8_t hh24, uint8_t mm);
    void disableINTA_Interrupts();
    void refreshINTA();
    void changeOffset(uint8_t digitalOffset);
protected:
    uint8_t intType, intPeriodicity, intHH24, intMM;
};

// DS1337 RTC
class DS1337: public RTC {
public:
	void begin(void);
	void adjust(const DateTime& dt); //Changes the date-time
	DateTime now(); //Gets the current date-time
	unsigned char time_is_set();
	unsigned char alarm_is_set();
	//unsigned char time_is_valid();

	void enable_interrupt();
	void disable_interrupt();
	void clear_interrupt();

	void readTime();
	void readAlarm();
	void writeTime();
	void writeAlarm();
	void writeAlarm(unsigned long sse);
	void setAlarmRepeat(byte repeat);
	unsigned char getSeconds();
	unsigned char getMinutes();
	unsigned char getHours();
	unsigned char getDays();
	unsigned char getDayOfWeek();
	unsigned char getMonths();
	unsigned int getYears();

	void setSeconds(unsigned char);
	void setMinutes(unsigned char);
	void setHours(unsigned char);
	void setDays(unsigned char);
	void setDayOfWeek(unsigned char);
	void setMonths(unsigned char);
	void setYears(unsigned int);
	void epoch_seconds_to_date(unsigned long seconds_left);
	unsigned long date_to_epoch_seconds(unsigned int year, byte month, byte day, byte hour, byte minute, byte second);
	unsigned long date_to_epoch_seconds();

	void start(void);
	void stop(void);
	unsigned char getRegister(unsigned char registerNumber);
	void setRegister(unsigned char registerNumber, unsigned char registerValue);

// library-accessible "private" interface
private:
	byte time_set;
	byte alarm_repeat;
	byte rtc_bcd[7]; // used prior to read/set DS1337 registers;
	void read(void);
	void save(void);
	byte bcd2bin(byte);
	byte bin2bcd(byte);

};


#endif

