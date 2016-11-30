/****************************************************
  nRF24l01+ Router

  The board should run with 3.3 V
  GPRS is directly powered from LIPO (3.7-4.2V)
  A mosfet switch will power down GPRS when
  LIPO voltage drops below a threshold

*****************************************************/

#include <BayEOSBuffer.h>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <BayEOS.h>
#include <Sleep.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Base64.h>
#include <SdFat.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>

#define NRF24_CHANNEL 0x72

#include <RF24.h>
RF24 radio(9, 10);
const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x96};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};

BayGPRS client = BayGPRS(Serial, 2);
uint8_t i2c_addresses[] = {0x50, 0x51, 0x52, 0x53};
BayEOSBufferMultiEEPROM myBuffer;

#define SAMPLING_INT 64
#define LCB_BAT_MULTIPLIER 1.1*540/100/1023
// we will collect 120 measurements before we try to send

#define ACTION_COUNT 2
#define LED_PIN 5
#include <LowCurrentBoard.h>
#include <RF24Router.h>
#define GPRS_POWER_PIN 8
#define POWER_OFF_LIMIT 3.8
#define POWER_ON_LIMIT 3.9

uint8_t res;
uint8_t status;

void setup()
{
  pinMode(GPRS_POWER_PIN, OUTPUT);
  digitalWrite(GPRS_POWER_PIN, HIGH);
  initLCB();
  //CHANGE CONFIG!!
  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|LP-GPRS|pinternet.interkom.de|||1812|"));
  blinkLED(2);
  res = client.begin(38400);
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

  /*
     1 == OK
     2 == no success
     3 == timeout
     4 == network timeout
     5 == gprs modem timeout
  */


  Wire.begin();
  myBuffer.init(4, i2c_addresses, 65536L, 0); //No flush
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  //We could also try to use absolute times received from GPRS!

  client.setBuffer(myBuffer);
  readBatLCB();
  startLCB();
  initRF24();
  status = 1;
}


void loop()
{
  handleRtcLCB();

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    client.startDataFrame(BayEOS_ChannelFloat32le);
    client.addChannelValue(millis(), 1);
    client.addChannelValue(batLCB, 2);
    client.addChannelValue(rx_ok, 3);
    client.addChannelValue(rx_error, 4);
    client.addChannelValue(tx_error, 5);
    client.writeToBuffer();
  }
  if (ISSET_ACTION(1)) {
    if (! status) { //GPRS is off
      readBatLCB();
      if (batLCB > POWER_ON_LIMIT) { //Has enough power
        digitalWrite(GPRS_POWER_PIN, HIGH);
        client.begin(38400);
        initRF24();
        status = 1;
      } else //power is not sufficient
        UNSET_ACTION(1);
    }

    if (status) { //GPRS is on
      if (res = client.sendMultiFromBuffer(2000)) tx_error++;
      else tx_error = 0;
      //Unset Action if all is sent or we got an sending error!
      if (! myBuffer.available() || tx_error > 5)
        UNSET_ACTION(1);
      blinkLED(res + 1);
      readBatLCB();
      if (batLCB < POWER_OFF_LIMIT) {
        handleRF24();
        radio.powerDown();
        digitalWrite(GPRS_POWER_PIN, LOW);
        Serial.end();
        pinMode(1, INPUT);
        status = 0;
        UNSET_ACTION(1);
      }
    }
  }

  if (status)
    handleRF24();
  sleepLCB();
}


