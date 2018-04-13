/*
 * Sketch for Testing the RTC clock
 * 
 * 
 * 
 */

//#define RTC_SECONDS_CORRECT -25000

#include <BaySerial.h>

BaySerial client = BaySerial(Serial);

#include <LowCurrentBoard.h>

void setup(void) {
  client.begin(38400);
  initLCB(); //init time2
  startLCB();
}

void loop(void) {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    client.startDataFrame();
    client.addChannelValue(myRTC.now().get());
    client.sendPayload();
  }
}

