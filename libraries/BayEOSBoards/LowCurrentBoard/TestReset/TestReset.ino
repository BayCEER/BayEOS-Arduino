/*
 * This is an example sketch for automatic reset
 * ACTION0_FLAG is not cleared for more than
 * RESET_COUNT*SAMPLING_INT
 */

#include <BayDebug.h>
BayDebug client(Serial);

#define RESET_COUNT 1
#define SAMPLING_INT 8

#include <LowCurrentBoard.h>

void setup(){
  initLCB();
  startLCB();
}

void loop(){
  if(ISSET_ACTION(0)){
    UNSET_ACTION(0);
  }
  delay(20000); //here a reset occures
}

