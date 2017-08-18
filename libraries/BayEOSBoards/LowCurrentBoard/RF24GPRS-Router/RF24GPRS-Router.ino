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
#include <Wire.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#endif

#include <BayEOS.h>
#include <Sleep.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Base64.h>
#include <SdFat.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>

#define NRF24_CHANNEL 0x43
#define NRF24_2CHANNEL 0x47
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

BayGPRS client = BayGPRS(Serial, 2);
#if FLASHSTORAGE
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
#else
uint8_t i2c_addresses[] = {0x50, 0x51, 0x52, 0x53};
BayEOSBufferMultiEEPROM myBuffer;
#endif

#define SAMPLING_INT 64
#define LCB_BAT_MULTIPLIER 1.1*540/100/1023
// we will collect 120 measurements before we try to send

#define LCB_BAT_MULTIPLIER 1.1*570/100/1023
#define ACTION_COUNT 2
#define LED_PIN 5
#include <LowCurrentBoard.h>
#include <RF24Router.h>
#define GPRS_POWER_PIN 7
#define POWER_OFF_LIMIT 3.7
#define POWER_ON_LIMIT 3.9

uint8_t res;
uint8_t status;

void setup()
{
  initLCB();
  initRF24();

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

  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|LP-RF24|pinternet.interkom.de|||4814|"));
  blinkLED(2);
  adjust_OSCCAL();
  res = client.begin(38400);
  if (! res) myRTC.adjust(client.now());
  blinkLED(res + 1);
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

  res = client.sendMessage("GPRS started");
  blinkLED(res + 1);
  delay(1000 + res * 500);

  /*
     1 == OK
     2 == no success
     3 == timeout
     4 == network timeout
     5 == gprs modem timeout
  */

  analogReference (INTERNAL);
  batLCB = LCB_BAT_MULTIPLIER * analogRead(A0);
  analogReference (DEFAULT);
  startLCB();
  status = 1;
}


void loop()
{
  handleRF24();
  checkAlive();

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    adjust_OSCCAL();  analogReference (INTERNAL);
    batLCB = LCB_BAT_MULTIPLIER * analogRead(A0);
    analogReference (DEFAULT);
    client.startDataFrameWithOrigin(BayEOS_Float32le, "GPRS", 0, 1);
    client.addChannelValue(batLCB, 30);
    client.addChannelValue(res);
    client.addChannelValue(client.getRSSI());
    client.addChannelValue(OSCCAL);
    client.writeToBuffer();


  }

  tx_blink = 0;
  checkSend();
  blinkLED(tx_blink);
  

}


