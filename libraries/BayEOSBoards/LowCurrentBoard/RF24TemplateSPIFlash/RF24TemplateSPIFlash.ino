#define SAMPLING_INT 32

#define RF24CHANNEL 0x66
#define RF24ADDRESS 0x45c431ae12LL

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 20); //use skip!
  initLCB(); //init time2
  readBatLCB();
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    client.startDataFrame(BayEOS_Int16le);
    client.addChannelValue(millis());
    client.addChannelValue(1000 * batLCB);
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



