//>>> The latest version of this code can be found at https://github.com/jcw/ !!

// A library for handling real-time clocks, dates, etc.
// 2010-02-04 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// $Id: RTClib.cpp 7763 2011-12-11 01:28:16Z jcw $
// A library for handling real-time clocks, dates, etc.
// 2010-02-04 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <avr/pgmspace.h>
#include "RTClib.h"
#include <Arduino.h>

#define DS1307_ADDRESS 0x68

#define PCF8563_ADDRESS 0x51
#define PCF8563_SEC_ADDR 0x02

/*Periodicity */
#define EverySecond   0
#define EveryMinute   1
#define EveryHour     2
#define EveryMonth    3

#define DS3231_ADDRESS	      0x68 //I2C Slave address

/* DS3231 Registers. Refer Sec 8.2 of application manual */
#define DS3231_SEC_REG        0x00
#define DS3231_MIN_REG        0x01
#define DS3231_HOUR_REG       0x02
#define DS3231_WDAY_REG       0x03
#define DS3231_MDAY_REG       0x04
#define DS3231_MONTH_REG      0x05
#define DS3231_YEAR_REG       0x06

#define DS3231_AL1SEC_REG     0x07
#define DS3231_AL1MIN_REG     0x08
#define DS3231_AL1HOUR_REG    0x09
#define DS3231_AL1WDAY_REG    0x0A

#define DS3231_AL2MIN_REG     0x0B
#define DS3231_AL2HOUR_REG    0x0C
#define DS3231_AL2WDAY_REG    0x0D

#define DS3231_CONTROL_REG          0x0E
#define DS3231_STATUS_REG           0x0F
#define DS3231_AGING_OFFSET_REG     0x0F
#define DS3231_TMP_UP_REG           0x11
#define DS3231_TMP_LOW_REG          0x12


#define PERIODIC_INTERRUPT	0x00
#define HM_INTERRUPT		0x01

#define RX8025_ADDRESS	      0x32 //I2C Slave address

/* RX8025 Registers. Refer Sec 8.2 of application manual */
#define RX8025_SEC_REG        0x00
#define RX8025_MIN_REG        0x10
#define RX8025_HOUR_REG       0x20
#define RX8025_WDAY_REG       0x30
#define RX8025_MDAY_REG       0x40
#define RX8025_MONTH_REG      0x50
#define RX8025_YEAR_REG       0x60
#define RX8025_DIGOFF_REG     0x70
#define RX8025_ALWMIN_REG     0x80
#define RX8025_ALWHOUR_REG    0x90
#define RX8025_ALWWDAY_REG    0xA0
#define RX8025_ALDMIN_REG     0xB0
#define RX8025_ALDHOUR_REG    0xC0
#define RX8025_RESERVED_REG   0xD0
#define RX8025_CTRL1_REG      0xE0
#define RX8025_CTRL2_REG      0xF0

#define RX8025_CTRL1_CT_BIT      (7 << 0)

/* periodic level interrupts used for powersaving mode of MCU*/
#define RX8025_CTRL1_CT_PER_OFF     0
#define RX8025_CTRL1_CT_PER_SEC     4
#define RX8025_CTRL1_CT_PER_MIN     5
#define RX8025_CTRL1_CT_PER_HOUR    6
#define RX8025_CTRL1_CT_PER_MONTH   7


//CTRL1 and CTLR2 register bits

#define RX8025_CTRL1_TEST_BIT    (1 << 3)
#define RX8025_CTRL1_1224_BIT    (1 << 5)
#define RX8025_CTRL1_DALE_BIT    (1 << 6)
#define RX8025_CTRL1_WALE_BIT    (1 << 7)

#define RX8025_CTRL2_DAFG_BIT    (1 << 0)
#define RX8025_CTRL2_WAFG_BIT    (1 << 1)
#define RX8025_CTRL2_CTFG_BIT    (1 << 2)
#define RX8025_CTRL2_CLEN1_BIT   (1 << 3)
#define RX8025_CTRL2_PON_BIT     (1 << 4)
#define RX8025_CTRL2_XST_BIT     (1 << 5)
#define RX8025_CTRL2_VDET_BIT    (1 << 6)
#define RX8025_CTRL2_VDSL_BIT    (1 << 7)


// indices within the rtc_bcd[] buffer
#define DS1337_SEC	0
#define DS1337_MIN	1
#define DS1337_HR	2
#define DS1337_DOW	3
#define DS1337_DATE     4
#define DS1337_MTH	5
#define DS1337_YR	6

#define DS1337_BASE_YR		2000

#define DS1337_CTRL_ID		B1101000

 // Define register bit masks
#define DS1337_CLOCKHALT	B10000000

#define DS1337_LO_BCD		B00001111
#define DS1337_HI_BCD		B11110000

#define DS1337_HI_SEC		B01110000
#define DS1337_HI_MIN		B01110000
#define DS1337_HI_HR		B00110000
#define DS1337_LO_DOW		B00000111
#define DS1337_HI_DATE		B00110000
#define DS1337_HI_MTH		B00110000
#define DS1337_HI_YR		B11110000

#define DS1337_ARLM1		0x07
#define DS1337_ARLM1_LO_SEC	B00001111
#define DS1337_ARLM1_HI_SEC	B01110000
#define DS1337_ARLM1_LO_MIN	B01110000
#define DS1337_ARLM1_HI_MIN	B00001111

#define DS1337_SP			0x0E
#define	DS1337_SP_EOSC		B10000000
#define	DS1337_SP_RS2		B00010000
#define	DS1337_SP_RS1		B00001000
#define	DS1337_SP_INTCN		B00000100
#define	DS1337_SP_A2IE		B00000010
#define	DS1337_SP_A1IE		B00000001

#define DS1337_STATUS		0x0F
#define DS1337_STATUS_OSF	B10000000
#define DS1337_STATUS_A2F	B00000010
#define DS1337_STATUS_A1F	B00000001

/* Definitions for alarm repeat */
/* The private variable alarm_repeat holds the user's alarm repeat preference. However, the DS1337 encodes these in the topmost bit(s) of the 4 alarm registers. */
/* Splattering these bits across the alarm regs is handled in the writeAlarm() function. */
/* If DY/DT is set, the day field is interpreted as a DayOfWeek (1 ~ 7), else it is interpreted as a DayOfMonth.*/

/* user alarm_repeat bit mask:
       7   6   5    4       3      2       1     0
      [x   x   x   A1M4   DY/DT   A1M3   A1M2   A1M1]
*/

#define DS1337_EVERY_SECOND       B00010111
#define DS1337_EVERY_MINUTE       B00010110
#define DS1337_EVERY_HOUR         B00010100
#define DS1337_EVERY_DAY          B00010000
#define DS1337_EVERY_WEEK         B00001000
#define DS1337_EVERY_MONTH        B00000000



////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }


////////////////////////////////////////////////////////////////////////////////
// RTC_DS1307 implementation

void RTC_DS1307::adjust(const DateTime& dt) {
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write((byte) 0);
    Wire.write(bin2bcd(dt.second()));
    Wire.write(bin2bcd(dt.minute()));
    Wire.write(bin2bcd(dt.hour()));
    Wire.write(bin2bcd(0));
    Wire.write(bin2bcd(dt.day()));
    Wire.write(bin2bcd(dt.month()));
    Wire.write(bin2bcd(dt.year() - 2000));
    Wire.write((byte) 0);
    Wire.endTransmission();
}

DateTime RTC_DS1307::now() {
   Wire.beginTransmission(DS1307_ADDRESS);
   Wire.write((byte) 0);
    Wire.endTransmission();

    Wire.requestFrom(DS1307_ADDRESS, 7);
    uint8_t ss = bcd2bin(Wire.read());
    uint8_t mm = bcd2bin(Wire.read());
    uint8_t hh = bcd2bin(Wire.read());
    Wire.read();
    uint8_t d = bcd2bin(Wire.read());
    uint8_t m = bcd2bin(Wire.read());
    uint16_t y = bcd2bin(Wire.read()) + 2000;
    
    return DateTime (y, m, d, hh, mm, ss);
}

///////////////////////////////////////////////////////////////////////////////
// RTC_PCF8563 implementation
// contributed by @mariusster, see http://forum.jeelabs.net/comment/1902

void RTC_PCF8563::adjust(const DateTime& dt) {
    Wire.beginTransmission(PCF8563_ADDRESS);
    Wire.write((byte) 0);
    Wire.write((byte) 0x0); // control/status1
    Wire.write((byte) 0x0); // control/status2
    Wire.write(bin2bcd(dt.second())); // set seconds
    Wire.write(bin2bcd(dt.minute())); // set minutes
    Wire.write(bin2bcd(dt.hour())); // set hour
    Wire.write(bin2bcd(dt.day())); // set day
    Wire.write((byte) 0x01); // set weekday
    Wire.write(bin2bcd(dt.month())); // set month, century to 1
    Wire.write(bin2bcd(dt.year() - 2000)); // set year to 00-99
    Wire.write((byte) 0x80); // minute alarm value reset to 00
    Wire.write((byte) 0x80); // hour alarm value reset to 00
    Wire.write((byte) 0x80); // day alarm value reset to 00
    Wire.write((byte) 0x80); // weekday alarm value reset to 00
    Wire.write((byte) 0x0); // set freqout 0= 32768khz, 1= 1hz
    Wire.write((byte) 0x0); // timer off
    Wire.endTransmission();
}

DateTime RTC_PCF8563::now() {
    Wire.beginTransmission(PCF8563_ADDRESS);
    Wire.write(PCF8563_SEC_ADDR);
    Wire.endTransmission();

    Wire.requestFrom(PCF8563_ADDRESS, 7);
    uint8_t ss = bcd2bin(Wire.read() & 0x7F);
    uint8_t mm = bcd2bin(Wire.read() & 0x7F);
    uint8_t hh = bcd2bin(Wire.read() & 0x3F);
    uint8_t d = bcd2bin(Wire.read() & 0x3F);
    Wire.read();
    uint8_t m = bcd2bin(Wire.read()& 0x1F);
    uint16_t y = bcd2bin(Wire.read()) + 2000;
    
    return DateTime (y, m, d, hh, mm, ss);
}

////////////////////////////////////////////////////////////////////////////////
// RTC_Timer2 implementation

void RTC_Timer2::adjust(const DateTime& dt) {
	noInterrupts();
    _seconds = dt.get();
    interrupts();
}

void RTC_Timer2::adjust(unsigned long sec) {
	noInterrupts();
    _seconds = sec;
    interrupts();
}

DateTime RTC_Timer2::now() {
	noInterrupts();
    unsigned long s=_seconds;
    interrupts();
    return s;
}

unsigned long RTC_Timer2::get() {
	noInterrupts();
    unsigned long s=_seconds;
    interrupts();
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// RTC_Millis implementation

long RTC_Millis::offset = 0;

void RTC_Millis::adjust(const DateTime& dt) {
	last_set=millis();
    offset = dt.get();
}

DateTime RTC_Millis::now() {
	if((millis()-last_set)>3456000000){
		adjust(now());
	}
    return offset + (millis()-last_set) / 1000;
}


////////////////////////////////////////////////////////////////////////////////
// RTC RX8025 implementation

void R8025::begin(void) {
  unsigned char ctReg1=0, ctReg2=0;
  Wire.beginTransmission(RX8025_ADDRESS);
  Wire.write((byte)RX8025_CTRL1_REG);     //CTRL1 Address
  // set the clock to 24hr format
  ctReg1 |= RX8025_CTRL1_1224_BIT;
  Wire.write((byte)ctReg1);
  //Set VDSL and XST bits. Also clear PON (power on reset) flag.
  Wire.write((byte)(RX8025_CTRL2_VDSL_BIT | RX8025_CTRL2_XST_BIT ));
  Wire.endTransmission();
  delay(10);

}

//Adjust the time-date specified in DateTime format
//writing any non-existent time-data may interfere with normal operation of the RTC
void R8025::adjust(const DateTime& dt) {

  Wire.beginTransmission(RX8025_ADDRESS);
  Wire.write((byte)RX8025_SEC_REG);  //beginning from SEC Register address

  Wire.write(bin2bcd(dt.second()));
  Wire.write(bin2bcd(dt.minute()));
  Wire.write(bin2bcd(dt.hour()));
  Wire.write(dt.dayOfWeek());
  Wire.write(bin2bcd(dt.day()));
  Wire.write(bin2bcd(dt.month()));
  Wire.write(bin2bcd(dt.year() - 2000));

  Wire.endTransmission();

}

//Read the current time-date and return it in DateTime format
DateTime R8025::now() {
  Wire.beginTransmission(RX8025_ADDRESS);
  Wire.write((byte)0x00);
  Wire.endTransmission();

  Wire.requestFrom(RX8025_ADDRESS, 8);
  Wire.read(); //ignore this data read from 0xF. Refer app note. Only Mode 3 Read operation
                  // is supported by wire library. i.e 0xF->0x0->0x2
  uint8_t ss = bcd2bin(Wire.read());
  uint8_t mm = bcd2bin(Wire.read());
  uint8_t hh = bcd2bin(Wire.read());
  uint8_t wd =  Wire.read();
  uint8_t d = bcd2bin(Wire.read());
  uint8_t m = bcd2bin(Wire.read());
  uint16_t y = bcd2bin(Wire.read()) + 2000;

  return DateTime (y, m, d, hh, mm, ss);
}

//Enable periodic interrupt at /INTA pin. Supports only the level interrupt
//for consistency with other /INTA interrupts. All interrupts works like single-shot counter
//Use refreshINTA() to re-enable interrupt.
void R8025::enableINTA_Interrupts(uint8_t periodicity)
{
  intType=PERIODIC_INTERRUPT; //Record interrupt type and periodicity for use of refreshINTA() method
  intPeriodicity=periodicity;

  uint8_t ctReg1= periodicity + 4;
  /*
  The above statement maps the periodicity defines from 0-3 into actual register value to be set for that mode.
  This arithmetic operation is simpler and faster than switch-case or if-else... Hence,

  EverySecond maps to RX8025_CTRL1_CT_PER_SEC
  EveryMinute maps to RX8025_CTRL1_CT_PER_MIN
  EveryHour  maps to RX8025_CTRL1_CT_PER_HOUR
  EveryMonth maps to RX8025_CTRL1_CT_PER_MONTH

*/
 //Maintain the clock in 24 Hour format
  ctReg1 |= RX8025_CTRL1_1224_BIT ;
  Wire.beginTransmission(RX8025_ADDRESS);
  Wire.write(RX8025_CTRL1_REG);
  Wire.write(ctReg1);
  Wire.endTransmission();
  delay(10);

  //set XST and VDSL flags.Clear all other flags
  Wire.beginTransmission(RX8025_ADDRESS);
  uint8_t ctReg2 = RX8025_CTRL2_XST_BIT | RX8025_CTRL2_VDSL_BIT;
  Wire.write(RX8025_CTRL2_REG);
  Wire.write(ctReg2);
  Wire.endTransmission();
  delay(10);
}

//Enable HH/MM interrupt on /INTA pin. All interrupts works like single-shot counter
//Use refreshINTA() to re-enable interrupt.
void R8025::enableINTA_Interrupts(uint8_t hh24, uint8_t mm)
{
   // hh24 is hours in 24 format (0-23). mm is in minutes(0 - 59)
   intType=HM_INTERRUPT; //Record interrupt type, hh24 and mm for use of refreshINTA() method
   intHH24 = hh24;
   intMM = mm;

   Wire.beginTransmission(RX8025_ADDRESS);
   Wire.write(RX8025_CTRL1_REG); //CTRL1 Address
  //Set DALE bit to 0 and take care of other bits of CTRL1
   Wire.write(RX8025_CTRL1_1224_BIT);
   Wire.endTransmission();

   Wire.beginTransmission(RX8025_ADDRESS);
   Wire.write(RX8025_ALDMIN_REG); //ALD Minute register Address
   Wire.write(bin2bcd(mm));
   Wire.write(bin2bcd(hh24));
   Wire.endTransmission();

   Wire.beginTransmission(RX8025_ADDRESS);
   Wire.write(RX8025_CTRL2_REG); //CTRL2 Address
   //Set DAFG bit to 0 and take care of other bits of CTRL2
   Wire.write(RX8025_CTRL2_VDSL_BIT | RX8025_CTRL2_XST_BIT );
   Wire.endTransmission();

   Wire.beginTransmission(RX8025_ADDRESS);
   Wire.write(RX8025_CTRL1_REG); //CTRL1 Address
  //Set DALE bit to 1 and take care of other bits of CTRL1
   Wire.write(RX8025_CTRL1_1224_BIT | RX8025_CTRL1_DALE_BIT);
   Wire.endTransmission();

}

//Disable Interrupts. This is equivalent to begin() method.
void R8025::disableINTA_Interrupts()
{
   begin(); //Restore the to initial value.
}

//This method must be called after an interrupt is detected. This refreshes the interrupt.
void R8025::refreshINTA()
{

 if(intType==PERIODIC_INTERRUPT)
 {
 //It is essential to clear CTFG flag to make /INTA high. Otherwise next interrupt will not be
 //detected by MCU. This is mentioned in page-19 timing-diagram of RX8025 App manual
 enableINTA_Interrupts(intPeriodicity); //Writing to CTRL2 is sufficient. But calling this method
                                        //is equilvalent for cleatring flag and setting Level interrupt.
 }
 else if(intType==HM_INTERRUPT)
 {
  enableINTA_Interrupts(intHH24, intMM);
  }
}

// Adjustment RTC Precision . Refer sec 8.3 of Application Manual
// This could be used only when required.
void R8025::changeOffset(uint8_t digitalOffset)
{
   // 'digitalOffset' can be any 7bit number
   Wire.beginTransmission(RX8025_DIGOFF_REG);
   Wire.write((digitalOffset & 0x7F));
   Wire.endTransmission();
}


uint8_t DS3231::readRegister(uint8_t regaddress)
{
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(regaddress);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 1);
    return Wire.read();
}

void DS3231::writeRegister(uint8_t regaddress,uint8_t value)
{
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(regaddress);
    Wire.write(value);
    Wire.endTransmission();
}

void DS3231::begin(void) {

  unsigned char ctReg=0;
  ctReg |= 0b00011100;
  writeRegister(DS3231_CONTROL_REG, ctReg);     //CONTROL Register Address
  delay(10);

  // set the clock to 24hr format
  uint8_t hrReg = readRegister(DS3231_HOUR_REG);
  hrReg &= 0b10111111;
  writeRegister(DS3231_HOUR_REG, hrReg);

  delay(10);
}

//Adjust the time-date specified in DateTime format
//writing any non-existent time-data may interfere with normal operation of the RTC
void DS3231::adjust(const DateTime& dt) {

  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_SEC_REG);  //beginning from SEC Register address

  Wire.write(bin2bcd(dt.second()));
  Wire.write(bin2bcd(dt.minute()));
  Wire.write(bin2bcd((dt.hour()) & 0b10111111)); //Make sure clock is still 24 Hour
  Wire.write(dt.dayOfWeek());
  Wire.write(bin2bcd(dt.day()));
  Wire.write(bin2bcd(dt.month()));
  Wire.write(bin2bcd(dt.year() - 2000));
  Wire.endTransmission();

}

//Read the current time-date and return it in DateTime format
DateTime DS3231::now() {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)0x00);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 8);
  uint8_t ss = bcd2bin(Wire.read());
  uint8_t mm = bcd2bin(Wire.read());

  uint8_t hrreg = Wire.read();
  uint8_t hh = bcd2bin((hrreg & ~0b11000000)); //Ignore 24 Hour bit

  uint8_t wd =  Wire.read();
  uint8_t d = bcd2bin(Wire.read());
  uint8_t m = bcd2bin(Wire.read());
  uint16_t y = bcd2bin(Wire.read()) + 2000;

  return DateTime (y, m, d, hh, mm, ss);
}
//Enable periodic interrupt at /INT pin. Supports only the level interrupt
//for consistency with other /INT interrupts. All interrupts works like single-shot counter
//Use refreshINTA() to re-enable interrupt.
void DS3231::enableInterrupts(uint8_t periodicity)
{

    unsigned char ctReg=0;
    ctReg |= 0b00011101;
    writeRegister(DS3231_CONTROL_REG, ctReg);     //CONTROL Register Address

   switch(periodicity)
   {
       case EverySecond:
       writeRegister(DS3231_AL1SEC_REG,  0b10000000 ); //set AM1
       writeRegister(DS3231_AL1MIN_REG,  0b10000000 ); //set AM2
       writeRegister(DS3231_AL1HOUR_REG, 0b10000000 ); //set AM3
       writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4

       break;

       case EveryMinute:
       writeRegister(DS3231_AL1SEC_REG,  0b00000000 ); //Clr AM1
       writeRegister(DS3231_AL1MIN_REG,  0b10000000 ); //set AM2
       writeRegister(DS3231_AL1HOUR_REG, 0b10000000 ); //set AM3
       writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4

       break;

       case EveryHour:
       writeRegister(DS3231_AL1SEC_REG,  0b00000000 ); //Clr AM1
       writeRegister(DS3231_AL1MIN_REG,  0b00000000 ); //Clr AM2
       writeRegister(DS3231_AL1HOUR_REG, 0b10000000 ); //Set AM3
       writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4

       break;
   }
}

//Enable HH/MM/SS interrupt on /INTA pin. All interrupts works like single-shot counter
void DS3231::enableInterrupts(uint8_t hh24, uint8_t mm, uint8_t ss)
{
    unsigned char ctReg=0;
    ctReg |= 0b00011101;
    writeRegister(DS3231_CONTROL_REG, ctReg);     //CONTROL Register Address

    writeRegister(DS3231_AL1SEC_REG,  0b00000000 | bin2bcd(ss) ); //Clr AM1
    writeRegister(DS3231_AL1MIN_REG,  0b00000000 | bin2bcd(mm)); //Clr AM2
    writeRegister(DS3231_AL1HOUR_REG, (0b00000000 | (bin2bcd(hh24) & 0b10111111))); //Clr AM3
    writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4
}

//Disable Interrupts. This is equivalent to begin() method.
void DS3231::disableInterrupts()
{
    begin(); //Restore to initial value.
}

//Clears the interrrupt flag in status register.
//This is equivalent to preparing the DS3231 /INT pin to high for MCU to get ready for recognizing the next INT0 interrupt
void DS3231::clearINTStatus()
{
    // Clear interrupt flag
    uint8_t statusReg = readRegister(DS3231_STATUS_REG);
    statusReg &= 0b11111110;
    writeRegister(DS3231_STATUS_REG, statusReg);

}

//force temperature sampling and converting to registers. If this function is not used the temperature is sampled once 64 Sec.
void DS3231::convertTemperature()
{
    // Set CONV
    uint8_t ctReg = readRegister(DS3231_CONTROL_REG);
    ctReg |= 0b00100000;
    writeRegister(DS3231_CONTROL_REG,ctReg);


    //wait until CONV is cleared. Indicates new temperature value is available in register.
    do
    {
       //do nothing
    } while ((readRegister(DS3231_CONTROL_REG) & 0b00100000) == 0b00100000 );

}

//Read the temperature value from the register and convert it into float (deg C)
float DS3231::getTemperature()
{
    int   temperatureCelsius;
    float fTemperatureCelsius;
    uint8_t tUBYTE  = readRegister(DS3231_TMP_UP_REG);  //Two's complement form
    uint8_t tLRBYTE = readRegister(DS3231_TMP_LOW_REG); //Fractional part

    if(tUBYTE & 0b10000000) //check if -ve number
    {
       tUBYTE  ^= 0b11111111;
       tUBYTE  += 0x1;
       fTemperatureCelsius = tUBYTE + ((tLRBYTE >> 6) * 0.25);
       fTemperatureCelsius = fTemperatureCelsius * -1;
    }
    else
    {
        fTemperatureCelsius = tUBYTE + ((tLRBYTE >> 6) * 0.25);
    }

    return (fTemperatureCelsius);

}


/********************************************************
 * DS1337
 *********************************************************/
// Cumulative number of days elapsed at the start of each month, assuming a normal (non-leap) year.
const unsigned int monthdays[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

// Aquire data from the RTC chip in BCD format
// refresh the buffer
void DS1337::readTime(void)
{
// use the Wire lib to connect to tho rtc
// reset the register pointer to zero
	Wire.beginTransmission(DS1337_CTRL_ID);
	Wire.write((uint8_t)0x00); // Explicit cast is to hack around http://code.google.com/p/arduino/issues/detail?id=527
	Wire.endTransmission();

// request the 7 bytes of data    (secs, min, hr, dow, date. mth, yr)
	Wire.requestFrom(DS1337_CTRL_ID, 7);
	for(int i=0; i<7; i++)
	{
	// store data in raw bcd format
		if (Wire.available())
			rtc_bcd[i]=Wire.read();
	}
}

// Read the current alarm value. Note that the repeat flags and DY/DT are removed from the result.
void DS1337::readAlarm(void)
{
        //alarm_repeat = 0;
        byte temp;
// use the Wire lib to connect to tho rtc
// point to start of Alarm1 registers
	Wire.beginTransmission(DS1337_CTRL_ID);
	Wire.write((uint8_t)DS1337_ARLM1);
	Wire.endTransmission();

// request the *4* bytes of data (secs, min, hr, dow/date). Note the format is nearly identical, except for the choice of dayOfWeek vs. date,
// and that the topmost bit of each helps determine if/how the alarm repeats.
	Wire.requestFrom(DS1337_CTRL_ID, 4);
	for(int i=0; i<4; i++)
	{
                // store data in raw bcd format
		if (Wire.available())
		{
			temp = Wire.read();
			rtc_bcd[i] = temp & B01111111;
		}
	}

	// 4th byte read may contain either a date or DayOfWeek, depending on the value of the DY/DT flag.
	// For laziness sake we read it into the DayOfWeek field regardless (rtc_bcd[3]). Correct as needed...
        if(rtc_bcd[3] & B01000000) // DY/DT set: DayOfWeek
        {
           rtc_bcd[3] &= B10111111; // clear DY/DT flag
           rtc_bcd[4] = 0; // alarm *date* undefined
        }
        else
        {
            rtc_bcd[4] = rtc_bcd[3];
            rtc_bcd[3] = 0; // alarm dayOfWeek undefined
        }
}

// update the data on the IC from the bcd formatted data in the buffer

void DS1337::writeTime(void)
{
	Wire.beginTransmission(DS1337_CTRL_ID);
	Wire.write((uint8_t)0x00); // reset register pointer
	for(int i=0; i<7; i++)
	{
		Wire.write(rtc_bcd[i]);
	}
	Wire.endTransmission();

	// clear the Oscillator Stop Flag
        setRegister(DS1337_STATUS, getRegister(DS1337_STATUS) & !DS1337_STATUS_OSF);
}


// FIXME: automatically set alarm interrupt after writing new alarm? Nah...

// Write the BCD alarm value in the buffer to the alarm registers.
// If an alarm repeat mode has been specified, poke those bytes into the buffer before sending.
void DS1337::writeAlarm(void)
{
	Wire.beginTransmission(DS1337_CTRL_ID);
	Wire.write((uint8_t)DS1337_ARLM1); // set register pointer

        Wire.write(rtc_bcd[DS1337_SEC] | ((alarm_repeat & B00000001 ) << 7)); // A1M1
        Wire.write(rtc_bcd[DS1337_MIN] | ((alarm_repeat & B00000010 ) << 6)); // A1M2
        Wire.write(rtc_bcd[DS1337_HR] | ((alarm_repeat & B00000100 ) << 5)); // A1M3

        // Check if we are using date or DayOfWeek and send the appropriate value
        if(alarm_repeat & B00001000) // DayOfWeek
        {
            // send DOW as 4th alarm reg byte
            Wire.write(rtc_bcd[DS1337_DOW] | ((alarm_repeat & B00011000 ) << 3)); // A1M4 and DY/DT
        }
        else // date
        {
            // send date as 4th alarm reg byte
            Wire.write(rtc_bcd[DS1337_DATE] | ((alarm_repeat & B00011000 ) << 3)); // A1M4 and DY/DT
        }

	Wire.endTransmission();
}


void DS1337::writeAlarm(unsigned long sse)
{
        epoch_seconds_to_date(sse);
        writeAlarm();
}


void DS1337::setAlarmRepeat(byte repeat)
{
        alarm_repeat = repeat;
}


unsigned char DS1337::getRegister(unsigned char registerNumber)
{
	Wire.beginTransmission(DS1337_CTRL_ID);
	Wire.write(registerNumber);
	Wire.endTransmission();

	Wire.requestFrom(DS1337_CTRL_ID, 1);

	return Wire.read();
}

void DS1337::setRegister(unsigned char registerNumber, unsigned char value)
{
	Wire.beginTransmission(DS1337_CTRL_ID);
	Wire.write(registerNumber); // set register pointer

	Wire.write(value);

	Wire.endTransmission();
}

unsigned char DS1337::time_is_set()
{
  // Return TRUE if Oscillator Stop Flag is clear (osc. not stopped since last time setting), FALSE otherwise
  byte asdf = ((getRegister(DS1337_STATUS) & DS1337_STATUS_OSF) == 0);
  return asdf;
}
unsigned char DS1337::alarm_is_set()
{
  // Return TRUE if the alarm interrupt flag is enabled.
  byte asdf = (getRegister(DS1337_SP) & DS1337_SP_A1IE);
  return asdf;
}

void DS1337::enable_interrupt()
{
   clear_interrupt();
   setRegister(DS1337_SP, getRegister(DS1337_SP) | DS1337_SP_INTCN | DS1337_SP_A1IE); // map alarm interrupt to INT1 and enable interrupt
}

void DS1337::disable_interrupt()
{
   setRegister(DS1337_SP, getRegister(DS1337_SP) & !DS1337_SP_A1IE);
}

void DS1337::clear_interrupt()
{
   setRegister(DS1337_STATUS, getRegister(DS1337_STATUS) & !DS1337_STATUS_A1F);
}

unsigned char DS1337::getSeconds()
{
    return bcd2bin(rtc_bcd[DS1337_SEC]);
}

unsigned char DS1337::getMinutes()
{
    return bcd2bin(rtc_bcd[DS1337_MIN]);
}
unsigned char DS1337::getHours()
{
    return bcd2bin(rtc_bcd[DS1337_HR]);
}
unsigned char DS1337::getDays()
{
    return bcd2bin(rtc_bcd[DS1337_DATE]);
}
unsigned char DS1337::getDayOfWeek()
{
    return bcd2bin(rtc_bcd[DS1337_DOW]);
}
unsigned char DS1337::getMonths()
{
    return bcd2bin(rtc_bcd[DS1337_MTH]);
}
unsigned int DS1337::getYears()
{
    return 2000 + bcd2bin(rtc_bcd[DS1337_YR]);
}



void DS1337::setSeconds(unsigned char v)
{
    rtc_bcd[DS1337_SEC] = bin2bcd(v);

}
void DS1337::setMinutes(unsigned char v)
{
    rtc_bcd[DS1337_MIN] = bin2bcd(v);

}
void DS1337::setHours(unsigned char v)
{
    rtc_bcd[DS1337_HR] = bin2bcd(v);

}
void DS1337::setDays(unsigned char v)
{
    rtc_bcd[DS1337_DATE] = bin2bcd(v);

}
void DS1337::setDayOfWeek(unsigned char v)
{
    rtc_bcd[DS1337_DOW] = bin2bcd(v);

}
void DS1337::setMonths(unsigned char v)
{
    rtc_bcd[DS1337_MTH] = bin2bcd(v);

}
void DS1337::setYears(unsigned int v)
{
    if (v>1999)
    {
        v -= 2000;
    }
    rtc_bcd[DS1337_YR] = bin2bcd(v);

}

byte DS1337::bcd2bin(byte v)
{
   return (v&0x0F) + ((v>>4)*10);
}

byte DS1337::bin2bcd(byte v)
{
   return ((v / 10)<<4) + (v % 10);
}

void DS1337::stop(void)
{
	setRegister(DS1337_SP, getRegister(DS1337_SP) | DS1337_SP_EOSC);
}

void DS1337::start(void)
{
	setRegister(DS1337_SP, getRegister(DS1337_SP) & !DS1337_SP_EOSC);
}

void DS1337::begin(void){
	start();
}
void DS1337::adjust(const DateTime& dt){
	setSeconds(dt.second());
	setMinutes(dt.minute());
	setHours(dt.hour());
	setDays(dt.day());
	setMonths(dt.month());
	setYears(dt.year());
	writeTime();
	; //Changes the date-time
}
DateTime DS1337::now(){
	readTime();
	return DateTime (getYears(), getMonths(), getDays(), getHours(), getMinutes(), getSeconds());
	; //Gets the current date-time
}

void DS1337::epoch_seconds_to_date(unsigned long seconds_left)
{
   // This routine taken from Dallas/Maxim application note 517
   // http://www.maxim-ic.com/app-notes/index.mvp/id/517
   // Arn't the fastest thing, but it produces correct results.

   // NOTE: The earliest date that can be represented by the DS1337 is 1/1/2000 (946684800 in Unix epoch seconds).
   // Passing an earlier Unix time stamp will fail quietly here (produce a date of 0/0/00), 
   // which will probably make your application angry.

   // ALSO NOTE: This has been optimized some to minimize redundant variables, with the side-effect
   // of making it much harder to understand. Please refer to the original appnote above
   // if you are trying to learn from it :-)


   //unsigned long hour;
   //unsigned long day;
   //unsigned long minute;
   //unsigned long second;
   unsigned long month;
   //unsigned long year;

	unsigned long seconds_left_2;
   //unsigned long whole_minutes;
   //unsigned long whole_hours;
   //unsigned long whole_days;
   //unsigned long whole_days_since_1968;
   unsigned long leap_year_periods;
   unsigned long days_since_current_lyear;
   //unsigned long whole_years;
   unsigned long days_since_first_of_year;
   unsigned long days_to_month;
   //unsigned long day_of_week;

   if(seconds_left >= 946684800)
   {
	   seconds_left -= 946684800; // correct for difference between DS1337 and UNIX epochs.

	   seconds_left_2 = seconds_left / 60; // seconds_left_2 = "whole_minutes"
	   rtc_bcd[DS1337_SEC] = bin2bcd(seconds_left - (60 * seconds_left_2));                 // leftover seconds

	   seconds_left = seconds_left_2 / 60; // seconds_left = "whole_hours"
	   rtc_bcd[DS1337_MIN] = bin2bcd(seconds_left_2 - (60 * seconds_left));            // leftover minutes

	   seconds_left_2 = seconds_left / 24; //seconds_left_2 = "whole_days"
	   rtc_bcd[DS1337_HR] = bin2bcd(seconds_left - (24 * seconds_left_2));         // leftover hours

	   //whole_days_since_1968 = whole_days;// + 365 + 366;	// seconds_left_2 = "whole_days" = "whole_days_since_1968"
	   leap_year_periods = seconds_left_2 / ((4 * 365) + 1);

	   days_since_current_lyear = seconds_left_2 % ((4 * 365) + 1);

	   // if days are after a current leap year then add a leap year period
	   if ((days_since_current_lyear >= (31 + 29))) {
		  leap_year_periods++;
	   }
	   seconds_left = (seconds_left_2 - leap_year_periods) / 365; // seconds_left = "whole_years"
	   days_since_first_of_year = seconds_left_2 - (seconds_left * 365) - leap_year_periods;

	   if ((days_since_current_lyear <= 365) && (days_since_current_lyear >= 60)) {
		  days_since_first_of_year++;
	   }
	   //year = seconds_left; // + 68;


		// seconds_left = "year"
		//seconds_left_2 = "month"
	   // walk across monthdays[] to find what month it is based on how many days have passed
	   //   within the current year
	   month = 13;
	   days_to_month = 366;
	   while (days_since_first_of_year < days_to_month) {
		   month--;
		   days_to_month = monthdays[month-1];
		   if ((month > 2) && ((seconds_left % 4) == 0)) {
			   days_to_month++;
			}
	   }
	   
	   rtc_bcd[DS1337_DATE] = bin2bcd( days_since_first_of_year - days_to_month + 1);

	   rtc_bcd[DS1337_DOW] = bin2bcd((seconds_left_2  + 4) % 7);


	   //rtc_bcd[DS1337_SEC] = bin2bcd(second);
	   //rtc_bcd[DS1337_MIN] = bin2bcd(minute);
	   //rtc_bcd[DS1337_HR] = bin2bcd(hour);
	   //rtc_bcd[DS1337_DATE] = bin2bcd(day);
	   //rtc_bcd[DS1337_DOW] = bin2bcd(day_of_week);
	   rtc_bcd[DS1337_MTH] = bin2bcd(month);
	   rtc_bcd[DS1337_YR] = bin2bcd(seconds_left);
   }
	else
	{
	// else: "invalid" (< year 2000) epoch format.
	// 'Best' way to handle this is to zero out the returned date. 
	
	   rtc_bcd[DS1337_SEC] = 0; //0x00 binary = 0x00 BCD
	   rtc_bcd[DS1337_MIN] = 0;
	   rtc_bcd[DS1337_HR] = 0;
	   rtc_bcd[DS1337_DATE] = 0;
	   rtc_bcd[DS1337_DOW] = 0;
	   rtc_bcd[DS1337_MTH] = 0;
	   rtc_bcd[DS1337_YR] = 0;
	}

}

unsigned long DS1337::date_to_epoch_seconds(unsigned int year, byte month, byte day, byte hour, byte minute, byte second)
{

  //gracefully handle 2- and 4-digit year formats
  if (year > 1999)
  {
     year -= 2000;
  }


// Between year 2000 and 2100, a leap year occurs in every year divisible by 4.

//   sse_y = (((unsigned long)year)*365*24*60*60);
//   sse_ly = ((((unsigned long)year+3)>>2) + ((unsigned long)year%4==0 && (unsigned long)month>2))*24*60*60;
//   sse_d = ((unsigned long)monthdays[month-1] + (unsigned long)day-1) *24*60*60;
//   sse_h = ((unsigned long)hour*60*60);
//   sse_m = ((unsigned long)minute*60);
//   sse_s = (unsigned long)second;
//
//   sse = sse_y + sse_ly + sse_d + sse_h + sse_m + sse_s;



// NB: The multiplication-by-constants below is intentionally left expanded for readability; GCC is smart and will optimize them to single constants during compilation.


  //         Whole year seconds                      Cumulative total of seconds contributed by elapsed leap year days
  unsigned long sse = (((unsigned long)year)*365*24*60*60)   +   ((((unsigned long)year+3)>>2) + ((unsigned long)year%4==0 && (unsigned long)month>2))*24*60*60   +   \
         ((unsigned long)monthdays[month-1] + (unsigned long)day-1) *24*60*60   +   ((unsigned long)hour*60*60)   +   ((unsigned long)minute*60)   + (unsigned long)second;
         // Seconds in days since start of year                      hours                      minutes           sec
  sse += 946684800; // correct for difference between DS1337 epoch and UNIX epoch
  return sse;
}

unsigned long DS1337::date_to_epoch_seconds()
{
     unsigned long asdf = date_to_epoch_seconds(int(bcd2bin(rtc_bcd[DS1337_YR])), bcd2bin(rtc_bcd[DS1337_MTH]), bcd2bin(rtc_bcd[DS1337_DATE]), bcd2bin(rtc_bcd[DS1337_HR]), bcd2bin(rtc_bcd[DS1337_MIN]), bcd2bin(rtc_bcd[DS1337_SEC]));
     return asdf;
}



