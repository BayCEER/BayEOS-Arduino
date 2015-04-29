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

#define TIMER0_ON 1
#define TIMER1_ON 2
#define TIMER2_ON 4

class SleepClass
{
public:
	/*
	 * 0-9:
	SLEEP_15Ms,
	SLEEP_30MS,
	SLEEP_60MS,
	SLEEP_120MS,
	SLEEP_250MS,
	SLEEP_500MS,
	SLEEP_1S,
	SLEEP_2S,
	SLEEP_4S,
	SLEEP_8S,
	 */
	static void setupWatchdog(int ii=6);

	/*
	 * NEED a RTC crystal!
	 *
	 * CS22	 CS21 	 CS20 	 DESCRIPTION
	 * 0	0 	0 	 Timer/Counter2 Disabled
	 * 0	0 	1 	 No Prescaling 1 -> 0,0078125sec
	 * 0	1 	0 	 Clock / 8  2 > 0,0625sec
	 * 0	1 	1 	 Clock / 32 3 -> 0,25sec
	 * 1	0 	0 	 Clock / 64 4 -> 0,5sec
	 * 1	0 	1 	 Clock / 128 5 -> 1sec
	 * 1	1 	0 	 Clock / 256 6 -> 2sec
	 * 1	1 	1 	 Clock / 1024 7 ->8sec
	 */
	static void setupTimer2(int ii=5);


	static void sleep(uint8_t modules=0);
};

extern SleepClass Sleep;


#endif
