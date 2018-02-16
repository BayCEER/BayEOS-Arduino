#define DALLAS_PIN 4
#define POWER_PIN 7
#define LED_PIN 5

#define TICKS_PER_SECOND 16
#define SAMPLING_INT 32

#define WITHDALLAS 0
#define WITHRAINGAUGE 0
#define NRF24_PIPE 0
#define RF24CHANNEL 0x66

#if NRF24_PIPE == 0
#define RF24ADDRESS 0x45c431ae12LL
#elif NRF24_PIPE == 1
#define RF24ADDRESS 0x45c431ae24LL
#elif NRF24_PIPE == 2
#define RF24ADDRESS 0x45c431ae48LL
#elif NRF24_PIPE == 3
#define RF24ADDRESS 0x45c431ae96LL
#elif NRF24_PIPE == 4
#define RF24ADDRESS 0x45c431aeabLL
#elif NRF24_PIPE == 5
#define RF24ADDRESS 0x45c431aebfLL
#endif


#include <BayEOSBufferRAM.h>
#include <BayRF24.h>
BayRF24 client=BayRF24(9,10);
BayEOSBufferRAM myBuffer;

#include <LowCurrentBoard.h>

void setup()
{
 client.init(RF24ADDRESS,RF24CHANNEL);
  myBuffer = BayEOSBufferRAM(1000);
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer,20); //use skip!
  initLCB(); //init time2   
  readBatLCB(); 
  startLCB();
}

void loop(){
  if(ISSET_ACTION(0)){
    UNSET_ACTION(0);
    //eg measurement
    client.startDataFrame(BayEOS_Int16le);
    client.addChannelValue(millis());
    client.addChannelValue(1000*batLCB);
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();
    
  }
  
  if(ISSET_ACTION(7)){
    UNSET_ACTION(7);
     client.sendFromBuffer();
  }
   sleepLCB();

}

