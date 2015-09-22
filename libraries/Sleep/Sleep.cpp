/*
  Sleep.cpp - Arduino Sleep library
  Copyright (c) arms22<arms22 at gmail.com>.  All right reserved.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  

*/

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#endif
#include "Sleep.h"

#include <avr/wdt.h>

void SleepClass::sleep(uint8_t modules,uint8_t sm){
//  power_adc_disable();
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
 // ACSR = (1<<ACD); //Disable the analog comparator
  if(!(modules & SPI_ON)) power_spi_disable();
  if(!(modules & TWI_ON)) power_twi_disable();
  if(!(modules & USART0_ON)) power_usart0_disable();
  if(!(modules & TIMER0_ON)) power_timer0_disable();
  if(!(modules & TIMER1_ON)) power_timer1_disable();
  if(!(modules & TIMER2_ON)) power_timer2_disable();

  set_sleep_mode(sm);
  cli();
  do{
	  sleep_enable();
#if defined __AVR_ATmega328P__
	  sleep_bod_disable();
#endif
	  sei();
	  sleep_cpu();     // System sleeps here
	  sleep_disable(); // System continues execution here when an interrupt woke up the divice
  } while(0);
  sei();
  power_all_enable();
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON

}


void SleepClass::setupTimer2(int ii) {
	TCCR2A = 0x00;
	if(ii>7) ii=7;
	TCCR2B = ii;
	ASSR = (1<<AS2); //Enable asynchronous operation
	TIMSK2 = (1<<TOIE2); //Enable the timer 2 interrupt
}

void SleepClass::setupWatchdog(int ii) {
/*	wdt_enable(ii);
	WDTCSR |= (1 << WDIE);

*/
  cbi( SMCR,SE );      // sleep enable, power down mode
  cbi( SMCR,SM0 );     // power down mode
  sbi( SMCR,SM1 );     // power down mode
  cbi( SMCR,SM2 );     // power down mode

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
//  Serial.println(ww);


  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);


}

SleepClass Sleep;
