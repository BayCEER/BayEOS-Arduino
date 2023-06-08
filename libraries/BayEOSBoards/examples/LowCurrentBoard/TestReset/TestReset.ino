/*
 * This is an example sketch for automatic reset
 * ACTION0_FLAG is not cleared for more than
 * RESET_COUNT*SAMPLING_INT
 */

#include <BayDebug.h>
BayDebug client(Serial);

#define RESET_COUNT 2
#define SAMPLING_INT 8

#include <LowCurrentBoard.h>

void setup(){
  initLCB();
  Serial.begin(9600);
  Serial.print("Setup: ");
  Serial.println(action0_pending_count);
  Serial.flush();
  startLCB();

}

void loop(){
  Serial.print("loop-1: ");
  Serial.println(action0_pending_count);
  Serial.flush();
  if(ISSET_ACTION(0)){
    UNSET_ACTION(0);
  }
  //During the loop a reset occures
  for (uint8_t i = 0; i < 30; i++) {
    Serial.print("loop-2: ");
    Serial.println(action0_pending_count);
    Serial.flush();
    delay(1000);
  }

}

