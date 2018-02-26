/*
  GBoard RF24-Router
  SPEC:
  SD:CS: 10 - 4
  GPRS: 6 - 46
  RF24: 8,9 - iBoardRF24
*/
#define GBoard 2
#define GBoardPro 3
//Set the board type here!!
//#define BOARD GBoard
#define BOARD GBoardPro
#define SKETCH_DEBUG 0

#define NRF24_CHANNEL 0x51
//#define NRF24_2CHANNEL 0x52
#define WITH_RF24_CHECKSUM 0


#define RX_LED A5
#define TX_LED A4
#define POWER_AD_PIN A3
#define POWER_DIVIDER (470.0+100.0)/100.0

#include <BayTCPSim900.h>
#include <BayEOSBufferSDFat.h>

#if (BOARD == GBoardPro)
#include <iBoardRF24.h>
iBoardRF24 radio(12, 11, 8, 7, 9, 2);
iBoardRF24 radio2(A15, A11, A12, A13, A14, 2);
//iBoardRF24 radio2(33,39,38,35,34, 2);
const uint64_t pipe_0 = 0x45c431ae12LL;
const uint64_t pipe_1 = 0x45c431ae24LL;
const uint64_t pipe_2 = 0x45c431ae48LL;
const uint64_t pipe_3 = 0x45c431ae96LL;
const uint64_t pipe_4 = 0x45c431aeabLL;
const uint64_t pipe_5 = 0x45c431aebfLL;
#define SD_CSPIN 4
#define GPRS_PIN 46
#define GPRS_RESET 47
#define TX_SERIAL Serial2
RTC_SIM900 myRTC;
#else
#include <RF24.h>
RF24 radio(8, 9);
const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x96};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};
#define SD_CSPIN 10
#define GPRS_PIN 6
#define GPRS_RESET 7
#define TX_SERIAL Serial
#endif

BayGPRS client = BayGPRS(TX_SERIAL, GPRS_PIN, GPRS_RESET);

#define SENDING_INTERVAL 120000L
#define NEXT_TRY_INTERVAL 300000L
#define MAX_BUFFER_AVAILABLE 2000


//BayDebug client;
BayEOSBufferSDFat myBuffer;

#include <RF24Router.h>


void setup(void) {
#if SKETCH_DEBUG
  Serial.begin(9600);
#endif
  initWatchdog();
  while (!SD.begin(SD_CSPIN)) {
    rx_blink = 2;
    tx_blink = 2;
    delay(2000);
  }
  client.readConfigFromFile("GPRS.TXT");

  startGPRS(0); 
  initRF24();
  myBuffer = BayEOSBufferSDFat(2000000000L, 1); //Append mode!

#if (BOARD == GBoardPro)
  myRTC.adjust(client.now());
  myBuffer.setRTC(myRTC, 0); //Relative Mode...
#endif
  client.setBuffer(myBuffer, 0);
  client.startFrame(BayEOS_Message);
  client.addToPayload("FW ");
  client.addToPayload(__DATE__);
  client.writeToBuffer();

  last_send -= SENDING_INTERVAL;
  last_alive -= SENDING_INTERVAL;
  last_try -= NEXT_TRY_INTERVAL;
  tx_error = 0;
#if SKETCH_DEBUG
  Serial.print("End Setup ");
  Serial.print(millis());
#endif

}

void loop(void) {
  wdreset = 1;
  checkAlive();
  checkSend();

  if (! handleRF24()) {
    delay(100);
  }

}


