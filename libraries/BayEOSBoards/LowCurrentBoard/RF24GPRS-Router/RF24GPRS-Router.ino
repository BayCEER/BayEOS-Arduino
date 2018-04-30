/****************************************************
  nRF24l01+ Router

  The board should run with 3.3 V
  GPRS is directly powered from LIPO (3.7-4.2V)
  A mosfet switch will power down GPRS when
  LIPO voltage drops below a threshold

*****************************************************/

#define FLASHSTORAGE 1

#include <BayEOSBuffer.h>

#if FLASHSTORAGE
#include <BayEOSBufferSPIFlash.h>
#else
#include <BayEOSBufferEEPROM.h>
#endif
#include <BayTCPSim900.h>

#define NRF24_CHANNEL 0x83
//#define NRF24_2CHANNEL 0x47
#define WITH_RF24_CHECKSUM 1



#include <RF24.h>
RF24 radio(9, 10);
#ifdef NRF24_2CHANNEL
RF24 radio2(6, 4);
#endif
const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x96};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};

BayGPRS client = BayGPRS(Serial, 0); //No Power Pin
#if FLASHSTORAGE
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
#else
uint8_t i2c_addresses[] = {0x50, 0x51, 0x52, 0x53};
BayEOSBufferMultiEEPROM myBuffer;
#endif

#define SAMPLING_INT 64
// we will collect 120 measurements before we try to send
#define BAT_MULTIPLIER 3.3*320/100/1023
#define ACTION_COUNT 2
#define LED_PIN 5
#include <LowCurrentBoard.h>
#include <RF24Router.h>
#define GPRS_POWER_PIN 7

#define SWITCH_OFF_MCP3424 1
#if SWITCH_OFF_MCP3424
#include <MCP342x.h>
MCP342x mcp342x = MCP342x();
const byte addr = 0;
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t gain = 0; //max Voltage: 0,512 Volt
#endif

void setup()
{
  initLCB();
  initRF24();

#if SWITCH_OFF_MCP3424
  mcp342x.setConf(0, 1, 0, 0, 0, 0);
  delayLCB(100);
  mcp342x.getData(addr);
#endif

#if FLASHSTORAGE

  myBuffer.init(flash, 10); //This will restore old pointers
#else
  Wire.begin();
  myBuffer.init(4, i2c_addresses, 65536L, 0); //No flush
#endif
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  //We could also try to use absolute times received from GPRS!
  client.setBuffer(myBuffer);
  //CHANGE CONFIG!!

  pinMode(GPRS_POWER_PIN, OUTPUT);
  digitalWrite(GPRS_POWER_PIN, HIGH);

  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|GPRS-T2R|pinternet.interkom.de|||9456|"));
  //  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|LP-RF24|pinternet.interkom.de|||4814|"));
  blinkLED(2);
  adjust_OSCCAL();
  tx_res = client.begin(38400);
  if (! tx_res) myRTC.adjust(client.now());
  blinkLED(tx_res + 1);
  /*
     1 == OK
     2 == NO Communication
     3 == PIN failed
     4 == PIN locked
     5 == Not CREG
     6 == Not CGATT
     7 == No SIM Card
  */
  delay(2000);

  tx_res = client.sendMessage("GPRS started");
  blinkLED(tx_res + 1);
  delay(1000 + tx_res * 500);

  /*
     1 == OK
     2 == no success
     3 == timeout
     4 == network timeout
     5 == gprs modem timeout
  */
  if (myRTC.now().get() < 2000) myBuffer.skip(); //got no time! skip the unread frames in Buffer!!

  startLCB();
}


void loop()
{
  handleRF24();

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    adjust_OSCCAL();
    analogReference (DEFAULT);
    batLCB = BAT_MULTIPLIER * analogRead(A7);
    client.startDataFrame();
    client.addChannelValue(millis() / 1000);
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myBuffer.readPos());
    client.addChannelValue(client.getRSSI());
    client.addChannelValue(batLCB);
    client.addChannelValue(tx_error);
    client.addChannelValue(tx_res);
    client.addChannelValue(rx1_count);
    client.addChannelValue(rx1_error);
    rx1_count = 0;
    rx1_error = 0;
#ifdef NRF24_2CHANNEL
    client.addChannelValue(rx2_count);
    client.addChannelValue(rx2_error);
    rx2_count = 0;
    rx2_error = 0;
#endif
    client.addChannelValue(OSCCAL);
    client.writeToBuffer();
    tx_res = client.sendMultiFromBuffer(1000);
    blinkLED(tx_res + 1);

    while (! tx_res && myBuffer.available()) {
      handleRF24();
      tx_res = client.sendMultiFromBuffer(1000);
      blinkLED(tx_res + 1);
    }
  }

  sleepLCB();

}


