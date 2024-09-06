#define RF24CHANNEL 0x66
#define RF24ADDRESS 0x45c431ae12LL
#define WITH_CHECKSUM 1
#define SAMPLING_INT 32
#define ACTION_COUNT 1
//#define LCB_BAT_MULTIPLIER 1.1*320/100/1023
//#define LCB_BAT_REFERENCE INTERNAL
//#define LCB_BAT_ADCPIN A0

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); // This will restore old pointers
  // myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip();                              // This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS); // use the rtc clock but relative
  client.setBuffer(myBuffer, 120);              // use skip!
  initLCB();                                    // init time2
  readBatLCB();
  startLCB();
}

void loop()
{
  if (ISSET_ACTION(0))
  {
    UNSET_ACTION(0);
    // measurement
    // Note we start a INT-Dataframe here
    // Only Values between -32768 and +32767 can be sent. You may have maximum 10 channels
    // Alternatively use 
    // client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
    // for float values. This limits channel to 5
    client.startDataFrame(BayEOS_Int16le, WITH_CHECKSUM);
    client.addChannelValue(millis());
    client.addChannelValue(1000 * batLCB);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();
    // Read battery voltage _after_ longer uptime!!!
    readBatLCB();
  }

  if (ISSET_ACTION(7))
  {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();
}
