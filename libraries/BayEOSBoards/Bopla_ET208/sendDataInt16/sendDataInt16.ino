#define SAMPLING_INT 32

//We use absolute origin frames in this sketch
//so RF24ADDRESS is not important for Origin. However
//one should cover all pipes.
//Do not use long names!! RF24 is limited to 32 byte!!!
//e.g: [DF][long][CF][RO][l][NAME][DF][T][offset][cpu][bat][temp][hum][light][cs]
//      1    4    1   1   1   5    1   1     1      2   2    2     2     2     2 = 28  
//MAXIMUM BOXNAME LENGTH: 9 !!!
#define BOXNAME "MB201"
#define RF24CHANNEL 0x5e
#define RF24ADDRESS 0x45e431aeab
#define RF24RETRY 3 /* max 15 */
#define RF24RETRYDELAY 10 /*7-15*/

#define WITHBUFFERINFO 0

#define PROTOTYPE_VERSION 0
#define SHT21 0


#include <BayEOSBufferSPIFlash.h>
#if PROTOTYPE_VERSION
#define I2C_SCL 3
#define I2C_SDA 4
#define POWER_DIVIDER ((470.0 + 100.0) / 100.0)
#else
#define I2C_SCL 2
#define I2C_SDA 3
#define POWER_DIVIDER ((1000.0 + 413.0) / 413.0)
#endif

#if SHT21
#include <Sensirion.h>
Sensirion sht(I2C_SDA, I2C_SCL, 0x40);
#else
#include <SHT3x.h>
SHT3x sht(I2C_SDA,I2C_SCL);
#endif

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

#define TICKS_PER_SECOND 4
#define ACTION_COUNT 1
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
  batLCB = mcp342x.getData() * POWER_DIVIDER;
#endif
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);

    client.startDataFrameWithOrigin(BayEOS_Int16le, BOXNAME, 1, 0);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB*1000);
    ret = sht.measureSleep(&temperature, &humidity);
    mcp342x.storeConf(3, 3); //18bit,8 gain
    mcp342x.runADC(0);
    delayLCB(mcp342x.getADCTime());
    span = mcp342x.getData();
    if(isnan(temperature)) temperature=-99.99;
    if(isnan(humidity)) humidity=-99.99;
    if(isnan(span)) span=-0.09999;
    client.addChannelValue(temperature*100); //Second Frame with Offset
    client.addChannelValue(humidity*100);
    client.addChannelValue(span*100000);
    client.addChecksum();
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    mcp342x.storeConf(1, 0); //14bit,0 gain
    mcp342x.runADC(2);
    delayLCB(mcp342x.getADCTime());
    batLCB = mcp342x.getData() * POWER_DIVIDER;

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



