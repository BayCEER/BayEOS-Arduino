#define SAMPLING_INT 32
// Set a unique board name
#define ORIGIN "Test-Board2"
// Set a startup delay to have the boards on the same bus
// out of sync
#define STARTUP_DELAY 10000

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BaySerial.h>
// Client with timeout 300ms, 38400 baud and CTS-pin 9
BaySerial client(Serial, 300, 38400, 9);

#include <LowCurrentBoard.h>

void setup()
{
  //Do not call client.begin(...) here
  //client.begin and client.end is automatically called by sendPayload()

  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 0); //do not use skip!
  initLCB(); //init time2
  delayLCB(STARTUP_DELAY);
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    client.startDataFrameWithOrigin(BayEOS_ChannelFloat32le, ORIGIN, 0, 1);
    client.addChannelValue(millis());
    client.addChannelValue(analogRead(0));
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}



