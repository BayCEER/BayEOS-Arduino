/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios. 
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two 
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting 
 * with the serial monitor and sending a 'T'.  The ping node sends the current 
 * time to the pong node, which responds by sending the value back.  The ping 
 * node can then see how long the whole cycle took.
 */
#include <BayEOS.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include <Sleep.h>
#include <BayRF24.h>
//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

BayRF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipe = 0x45c431ae12LL;

volatile uint8_t kippevent=0;
volatile uint8_t wdcount=0; 

ISR(WDT_vect) {
  wdcount++;
}

int kippcount;
void kippwaage(){
  kippevent=1;
}

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
//

// The various roles supported by this sketch

void setup(void)
{
  Sleep.setupWatchdog(8); //init watchdog timer to 4 sec
  attachInterrupt(0, kippwaage, FALLING);
  radio.init(pipe);
  digitalWrite(2,HIGH); //INT0
  analogReference(EXTERNAL);
 
}

void loop(void)
{
  if(kippevent){
    delay(200);
    kippevent=0;
    kippcount++;
  }
  //
  // Ping out role.  Repeatedly send the current time
  //

  // First, stop listening so we can talk.
  if(wdcount>1){
   pinMode(A4,OUTPUT);
   digitalWrite(A4,HIGH);
   radio.startDataFrame(BayEOS_Int16le);
   for(uint8_t i=0;i<4;i++){
     radio.addChannelValue(analogRead(A0+i));
   }
   radio.addChannelValue(analogRead(A5));
   radio.addChannelValue(kippcount);
   pinMode(A4,INPUT);
   digitalWrite(A4,LOW);
   
  radio.sendPayload();
  wdcount=0;
  }
  Sleep.sleep();

}


