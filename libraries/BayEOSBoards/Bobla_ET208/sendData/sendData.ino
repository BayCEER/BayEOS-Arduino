#define SAMPLING_INT 32

//We use absolute origin frames in this sketch
//so RF24ADDRESS is not important for Origin. However
//one should cover all pipes.
//Do not use long names!! RF24 is limited to 32 byte!!!
//e.g: [DF][long][CF][RO][l][NAME][DF][T][offset][temp][hum][light][cs]
//      1    4    1   1   1   5    1   1     1       4     4    4    2 = 30
//MAXIMUM BOXNAME LENGTH: 7 !!!
#define BOXNAME "MB201"
#define RF24CHANNEL 0x7e
#define RF24ADDRESS 0x45c431ae12LL
#define RF24RETRY 3 /* max 15 */
#define RF24RETRYDELAY 10 /*7-15*/

#define WITHBUFFERINFO 1
#define WITHBATINFO 1

#include <BayEOSBufferSPIFlash.h>
#include <Sensirion.h>
Sensirion sht2 = Sensirion(4, 3, 0x40);
float temperature;
float humidity;
int ret;
#include <MCP342x.h>
MCP342x mcp342x(0);
float span;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);
  client.setRetries(RF24RETRY, RF24RETRYDELAY);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 100); //use skip!
  initLCB(); //init time2
  mcp342x.reset();
#if WITHBATINFO
  mcp342x.storeConf(1, 0); //14bit,0 gain
  mcp342x.runADC(2);
  delayLCB(mcp342x.getADCTime());
  batLCB = mcp342x.getData() * (470.0 + 100.0) / 100.0;
#endif
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);

#if WITHBATINFO
    client.startDataFrameWithOrigin(BayEOS_Float32le, BOXNAME, 1, 0);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB);
    client.addChecksum();
    client.sendOrBuffer();
#endif
    ret = sht2.measureSleep(&temperature, &humidity);
    mcp342x.storeConf(3, 3); //18bit,8 gain
    mcp342x.runADC(0);
    delayLCB(mcp342x.getADCTime());
    span = mcp342x.getData();
    client.startDataFrameWithOrigin(BayEOS_Float32le, BOXNAME, 1, 0);
    client.addChannelValue(temperature, 3); //Second Frame with Offset
    client.addChannelValue(humidity);
    client.addChannelValue(span);
    client.addChecksum();
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
#if WITHBATINFO
    mcp342x.storeConf(1, 0); //14bit,0 gain
    mcp342x.runADC(2);
    delayLCB(mcp342x.getADCTime());
    batLCB = mcp342x.getData() * (470.0 + 100.0) / 100.0;
#endif

#if WITHBUFFERINFO
    client.startDataFrameWithOrigin(BayEOS_Float32le, BOXNAME, 1, 0);
    client.addChannelValue(myBuffer.readPos(), 6);
    client.addChannelValue(myBuffer.writePos());
    client.addChecksum();
    client.sendOrBuffer();
#endif

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}



