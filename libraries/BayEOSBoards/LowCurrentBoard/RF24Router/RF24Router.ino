/****************************************************************

   Like RF24RainMultiDallasEEPROM.ino
   but additionally acts as RF24-RF24-Router

NOTE: This scetch works but power consumption is 15mA due to
keeping nRF24l01+ in listening mode!!!


 ***************************************************************/
#define DALLAS_PIN A1
#define POWER_PIN 7
#define LED_PIN 5

// 16 ticks per second!
#define RAINGAUGE_LAGTICKS 12
#define SAMPLING_INTTICKS 512
#define WITHDALLAS 0
#define WITHRAINGAUGE 0
#define WITHSHT 0
//#define RF24ADDRESS 0x45c431ae12LL
#define RF24ADDRESS 0x45c431ae48LL
#define RF24CHANNEL 0x61

#include <BayEOSBufferEEPROM.h>
#include <SHT2xSleep.h>
#include <BayRF24.h>


BayRF24 client = BayRF24(9, 10, 0); //no power down!
BayEOSBufferEEPROM myBuffer;


const uint64_t pipes[6] = { 0x85c831a312LL, 0x85c831a324LL, 0x85c831a348LL,
                            0x85c831a396LL, 0x85c831a3abLL, 0x85c831a3bfLL
                          };
/*
  const uint64_t pipes[6] = { 0x45c431ae13LL, 0x45c431ae25LL, 0x45c431ae49LL,
                            0x45c431ae9eLL, 0x45c431aeaaLL, 0x45c431aebeLL
                          };
*/

//include some functions for low current board
//expects BayEOS-Client to be called "client"

#include <LowCurrentBoard.h>

float temp, hum;


void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);
  for (uint8_t i = 1; i < 6; i++) {
    client.openReadingPipe(i, pipes[i]);
  }
  client.startListening();

  Wire.begin();
  myBuffer.init(0x50, 65536L, 0); //NO flush!!
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 100); //use skip!
  initLCB(); //init time2
  readBatLCB();
  startLCB();


}

void loop()
{
  handleRtcLCB();
  handleRF24();

#if WITHDALLAS
  //Do conversion 32 ticks (2sec) bevor sampling!
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    ds.t_conversion();
  }
  if (ISSET_ACTION(1)) {
    UNSET_ACTION(1);
    readAndSendDallasLCB();
    readBatLCB();
  }
#endif

#if WITHRAINGAUGE
  handleRainEventLCB();
#endif


  // Measure and send
  if (ISSET_ACTION(2)) {
    UNSET_ACTION(2);
    client.startDataFrame(BayEOS_ChannelFloat32le);
    client.addChannelValue(millis(), 1);
    client.addChannelValue(batLCB, 2);
#if WITHSHT
    hum = SHT2x.GetHumidity();
    temp = SHT2x.GetTemperature();
    SHT2x.reset();
    client.addChannelValue(temp, 3);
    client.addChannelValue(hum, 4);
#endif
#if WITHRAINGAUGE
    client.addChannelValue(rain_count, 5);
#endif
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();

  }

  // Resend from Buffer
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);

#if SERIALDEBUG
    Serial.println(myBuffer.available());
    delay(20);
#endif
    client.sendFromBuffer();
  }

  sleepLCB();
}

void handleRF24(void) {
  uint8_t pipe_num, len;
  uint8_t payload[32];
  if (client.i_available(&pipe_num)) {
    client.startRoutedFrame(pipe_num, 0);
    len = client.getDynamicPayloadSize();
    // Fetch the payload
    client.read( payload, len );
    for (uint8_t i = 0; i < len; i++) {
      client.addToPayload(payload[i]);
    }
    client.sendOrBuffer();
  }

}



