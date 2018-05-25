//Change RF Channel and Address
#define RF24CHANNEL 0x47
#define RF24ADDRESS 0x45c431ae12LL

#define SAMPLING_INT 32
#define ACTION_COUNT 1
#define WITH_CHECKSUM 1
#define MCPPOWER_PIN 6
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR 0.5

#define PRE_RESISTOR 14300


//Set this to 1 to get BayDebug Output!
#define SKETCH_DEBUG 0


#include <BayEOSBufferSPIFlash.h>
#include <MCP342x.h>

const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 3; //0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);

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

float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}


void setup()
{

#if SKETCH_DEBUG
  client.begin(9600, 1);
#else
  client.init(RF24ADDRESS, RF24CHANNEL);
#endif
  myBuffer.init(flash, 10); //This will restore old pointers
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  myBuffer.reset();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  client.setBuffer(myBuffer, 100); //use skip!
  initLCB(); //init time2
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(MCPPOWER_PIN, OUTPUT);
  readBatLCB();
  startLCB();
}

void loop()
{
  float span, strom, ntc10r;
  // Measure and send
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    digitalWrite(MCPPOWER_PIN, HIGH);
    client.startDataFrame(BayEOS_Int16le, WITH_CHECKSUM);
    client.addChannelValue(1000 * batLCB);
    for (uint8_t ch = 0; ch < 8; ch++) {
      digitalWrite(A1, ch & 0x4);
      digitalWrite(A2, ch & 0x2);
      digitalWrite(A3, ch & 0x1);
      delayLCB(10);
      mcp342x.runADC(0);
      delayLCB(mcp342x.getADCTime());
      span = mcp342x.getData();
      strom = span / PRE_RESISTOR;

      mcp342x.runADC(1);
      delayLCB(mcp342x.getADCTime());
      span = mcp342x.getData();
      ntc10r = span / strom / NTC10FACTOR;
      client.addChannelValue(1000 * ntc10_R2T(ntc10r));
    }
    digitalWrite(MCPPOWER_PIN, LOW);

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


