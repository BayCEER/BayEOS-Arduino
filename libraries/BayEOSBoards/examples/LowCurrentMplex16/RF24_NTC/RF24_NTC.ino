//Change RF Channel and Address
#define RF24CHANNEL 0x7e
#define RF24ADDRESS 0x45c431aebfLL

#define SAMPLING_INT 32
#define ACTION_COUNT 1
#define WITH_CHECKSUM 1
#define MCPPOWER_PIN 6
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR 0.5

#define PRERESISTOR 10000.0

//When inividual preresistorvalues are given PRERESITOR is ignored
//#define PRERESISTORS {9955.0, 9964.0, 9956.0, 9966.0, 9955.0, 9972.0, 9975.0, 9972.0, 9945.0, 9962.0, 9988.0, 9957.0, 9957.0, 9964.0, 9950.0, 9954.0 }

//Define resolution
const uint8_t rate = 2; //0-3: 12bit ... 18bit

//Set this to 1 to get BayDebug Output!
#define SKETCH_DEBUG 0


#include <BayEOSBufferSPIFlash.h>
#include <MCP342x.h>

const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
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


void readBat(void) {
  analogReference (DEFAULT);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  batLCB = 3300.0 * analogRead(A7) / 1023 * (100.0 + 100.0) / 100.0;
  digitalWrite(POWER_PIN, LOW);
  pinMode(POWER_PIN, INPUT);

}


void setup()
{

#if SKETCH_DEBUG
  client.begin(9600, 1);
#else
  client.init(RF24ADDRESS, RF24CHANNEL);
#endif
  myBuffer.init(flash); //This will restore old pointers
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  client.setBuffer(myBuffer, 100); //use skip!
  initLCB(); //init time2
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(MCPPOWER_PIN, OUTPUT);
  readBat();
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
    client.addChannelValue(millis());
    client.addChannelValue(batLCB);
    delayLCB(20); //wait for capacities to be loaded.

    for (uint8_t ch = 0; ch < 16; ch++) {
      if ((ch & 0x8) == 0) {
#if WITH_CHECKSUM
        client.addChecksum();
#endif
        client.sendOrBuffer();
        client.startDataFrame(BayEOS_Int16le, WITH_CHECKSUM);
      }
      digitalWrite(A0, ch & 0x8);
      digitalWrite(A1, ch & 0x4);
      digitalWrite(A3, ch & 0x2);
      digitalWrite(A2, ch & 0x1);
      delayLCB(10);
      mcp342x.runADC(0);
      delayLCB(mcp342x.getADCTime());
      span = mcp342x.getData();
#ifdef PRERESISTORS
      strom = span / preresistors[ch]; //current in A
#else
      strom = span / PRERESISTOR; //current in A
#endif

      mcp342x.runADC(1);
      delayLCB(mcp342x.getADCTime());
      span = mcp342x.getData();
      ntc10r = span / strom / NTC10FACTOR;
      client.addChannelValue(1000 * ntc10_R2T(ntc10r), ch + 3);
    }
    digitalWrite(MCPPOWER_PIN, LOW);

#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();
    readBat();

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
