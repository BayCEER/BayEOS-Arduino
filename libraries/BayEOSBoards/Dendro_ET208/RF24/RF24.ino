/***
 * Wiring
 * white - GND
 * brown - SIG
 * green - PW
 * 
 */

#define RF24CHANNEL 0x57
//#define RF24ADDRESS 0x4ec431ae12LL
//#define RF24ADDRESS 0x4ec431ae24LL
//#define RF24ADDRESS 0x4ec431ae48LL
//#define RF24ADDRESS 0x4ec431ae96LL
#define RF24ADDRESS 0x4ec431aeabLL
#define WITH_CHECKSUM 1
#define TOTAL_LENGTH 10

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <DendroET208.h>

#define ACTION_COUNT 1
#define SAMPLING_INT 64
#define LCB_BAT_ADCPIN A7
#include <LowCurrentBoard.h>

void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 20); //use skip!
  initLCB(); //init time2
  initDendro();
  readBatLCB();
  startLCB();

}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    adc.read(1); //read with calibration
    delayLCB(40);
    //Note we start a INT-Dataframe here
    //Only Values between -32768 and +32767 can be sent
    client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB*1000);
    client.addChannelValue((float)readDendro()*1000*TOTAL_LENGTH);

#if WITH_CHECKSUM
    client.addChecksum();
#endif
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
