#define NRF24_PIPE 0
#define RF24CHANNEL 0x47
#define SAMPLING_INT 32
#define ACTION_COUNT 1
#define WITH_CHECKSUM 1


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

//Set this to 1 to get BayDebug Output!
#define SKETCH_DEBUG 0


#include <BayEOSBuffer.h>
#include <Wire.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <MCP342x.h>

const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
//  create an objcet of the class MCP342x
MCP342x mcp342x = MCP342x();

#if SKETCH_DEBUG
#include <BayDebug.h>
BayDebug client(Serial);
#else
#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);
#endif

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;


#include <LowCurrentBoard.h>


void setup()
{

  Wire.begin();
#if SKETCH_DEBUG
  client.begin(9600, 1);
#else
  client.init(RF24ADDRESS, RF24CHANNEL);
#endif
  myBuffer.init(flash, 10); //This will restore old pointers
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  // myBuffer.reset();
  client.setBuffer(myBuffer, 100); //use skip!
  initLCB(); //init time2
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  readBatLCB();
  startLCB();
}

void loop()
{
  float span, strom;
  // Measure and send
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
    for (uint8_t ch = 0; ch < 8; ch++) {
      digitalWrite(A1, ch & 0x4);
      digitalWrite(A2, ch & 0x2);
      digitalWrite(A3, ch & 0x1);
      mcp342x.setConf(addr, 1, 0, mode, rate, gain);
      delayLCB(100);
      span = mcp342x.getData(addr);
      strom = span / 14.3;
      mcp342x.setConf(addr, 1, 1, mode, rate, gain);
      delayLCB(100);
      span = mcp342x.getData(addr);
      client.addChannelValue(span / strom, ch+1);
      if (ch == 3) {
#if WITH_CHECKSUM
        client.addChecksum();
#endif
        client.sendOrBuffer();
        client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
      }

    }
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();
    readBatLCB();

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);

#if SERIALDEBUG
    Serial.println(myBuffer.available());
    delay(20);
#endif

    client.sendFromBuffer();
  }

#if SKETCH_DEBUG
  delay(50);
#else
  sleepLCB();
#endif


}


