//>>> The latest version of this code can be found at https://github.com/jcw/ !!

// A library for handling real-time clocks, dates, etc.
// 2010-02-04 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// $Id: RTClib.cpp 7763 2011-12-11 01:28:16Z jcw $
// A library for handling real-time clocks, dates, etc.
// 2010-02-04 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <Wire.h>
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
// RTC_Millis implementation

long RTC_Millis::offset = 0;

void RTC_Millis::adjust(const DateTime& dt) {
    offset = dt.get() - millis() / 1000;
}

DateTime RTC_Millis::now() {
    return offset + millis() / 1000;
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




