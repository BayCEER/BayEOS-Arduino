/*
  Sleep.h - Arduino Sleep library
  Copyright (c) arms22<arms22 at gmail.com>.  All right reserved.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  

*/

#ifndef Sleep_h
#define Sleep_h

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <inttypes.h>
#include <avr/wdt.h>


class SleepClass
{
public:
	static void setupWatchdog(int ii);
	static void sleep(void);
};

extern SleepClass Sleep;


#endif
