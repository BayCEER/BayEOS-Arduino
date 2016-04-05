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

#define NRF24_CHANNEL 0x72

#define RX_LED A2
#define TX_LED A1

#include <EEPROM.h>
#include <BayEOS.h>
#include <SPI.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <Sleep.h>
#include <SoftwareSerial.h>
#if (BOARD == GBoardPro)
#include <iBoardRF24.h>
iBoardRF24 radio(12, 11, 8, 7, 9, 2);
const uint64_t pipe_0 = 0x45c431ae12LL;
const uint64_t pipe_1 = 0x45c431ae24LL;
const uint64_t pipe_2 = 0x45c431ae48LL;
const uint64_t pipe_3 = 0x45c431ae9fLL;
const uint64_t pipe_4 = 0x45c431aeabLL;
const uint64_t pipe_5 = 0x45c431aebfLL;
#define SD_CSPIN 4
#define GPRS_PIN 46
#define TX_SERIAL Serial2
RTC_SIM900 myRTC;
#else
#include <RF24.h>
RF24 radio(8, 9);
const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x9f};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbe};
#define SD_CSPIN 10
#define GPRS_PIN 6
#define TX_SERIAL Serial
#endif

BayGPRS client = BayGPRS(TX_SERIAL, GPRS_PIN);

#define SENDING_INTERVAL 120000

unsigned long last_alive, last_send, pos, last_eeprom;
uint16_t rx_ok, rx_error, tx_error;
uint8_t rep_tx_error, tx_res;
uint8_t last_rx_rssi;
uint8_t startupframe, startupsend;

//BayDebug client;
BayEOSBufferSDFat myBuffer;
extern volatile unsigned long timer0_millis;

volatile uint16_t wdcount = 0;
volatile uint8_t wdreset = 0;
volatile uint8_t tx_blink = 0;
volatile uint8_t rx_blink = 0;
volatile uint8_t rx_on = 0;
volatile uint8_t tx_on = 0;
volatile uint16_t wdt_millis;
uint16_t sleep_time;
uint32_t total_sleep_time;
ISR(WDT_vect) {
  wdcount++;
  wdt_millis=millis();
  if (wdreset) wdcount = 0;
  if (wdcount > 480) { //no action for more than 120 sec.
    asm volatile (" jmp 0"); //restart programm
  }
  if (tx_blink) {
    if (tx_on) {
      digitalWrite(TX_LED, LOW);
      tx_on = 0;
      tx_blink--;
    } else {
      digitalWrite(TX_LED, HIGH);
      tx_on = 1;
    }
  }
  if (rx_blink) {
    if (rx_on) {
      digitalWrite(RX_LED, LOW);
      rx_on = 0;
      rx_blink--;
    } else {
      digitalWrite(RX_LED, HIGH);
      rx_on = 1;
    }
  }
}


void initRF24(void){
  radio.powerUp();
  radio.setChannel(NRF24_CHANNEL);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15,15);
  radio.setAutoAck(true);
//  radio.openWritingPipe(pipe_0);
  radio.openReadingPipe(0, pipe_0);
  radio.openReadingPipe(1, pipe_1);
  radio.openReadingPipe(2, pipe_2);
  radio.openReadingPipe(3, pipe_3);
  radio.openReadingPipe(4, pipe_4);
  radio.openReadingPipe(5, pipe_5);
  
  radio.startListening();
}


void setup(void) {

  if (!SD.begin(SD_CSPIN)) {
    return;
  }
  pinMode(RX_LED, OUTPUT);
  pinMode(TX_LED, OUTPUT);

  client.readConfigFromFile("GPRS.TXT");
  client.begin(38400);

  myBuffer = BayEOSBufferSDFat(2000000000L, 1); //Append mode!
#if (BOARD == GBoardPro)
//  myRTC.adjust(client.now());
//  myBuffer.setRTC(myRTC, 0); //Relative Mode...
#endif
  client.setBuffer(myBuffer, 0);

  client.startFrame(BayEOS_Message);
  client.addToPayload("FW ");
  client.addToPayload(__DATE__);
  client.writeToBuffer();

  Sleep.setupWatchdog(4); //250ms
  radio.begin();
  initRF24();
}

void loop(void) {
  wdreset = 1;
  if ((millis() - last_alive) > SENDING_INTERVAL || startupframe) {
    startupframe = 0;
    last_alive = millis();
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myBuffer.readPos());
    client.addChannelValue(client.getRSSI());
    client.addChannelValue((float)analogRead(A4) / 1023 * 3.3 * 10);
    client.addChannelValue(total_sleep_time / 1000);
    client.writeToBuffer();
  }

  if (    ((millis() - last_send) > SENDING_INTERVAL ||
           myBuffer.available() > 2000 || startupsend)) {
    last_send = millis();
    radio.stopListening();
    
    if ( (tx_res = client.sendMultiFromBuffer()) ) {
      tx_error++;
      rep_tx_error++;
    } else {
      rep_tx_error = 0;
      startupsend = 0;

      if (rep_tx_error % 5 == 4) {
        client.softSwitch();
        client.startFrame(BayEOS_Message);
        client.addToPayload("TX-ERROR SoftSwitch");
        client.writeToBuffer();
      }
    }
    radio.startListening();
    //initRF24();

  }
  handle_RF24();
  noInterrupts();
  sleep_time=250 - ((uint16_t)millis() - wdt_millis);
  interrupts();
  if(sleep_time>250) sleep_time=250;
  Sleep.sleep();
  noInterrupts();
  timer0_millis+=sleep_time;
  interrupts();
  total_sleep_time+=sleep_time;
#if (BOARD == GBoardPro)
//  myRTC.adjust(client.now());
#endif

}

void handle_RF24(void) {
  uint8_t pipe_num, len;
  uint8_t payload[32];
  uint8_t count;
  while (radio.available(&pipe_num)) {
    count++;
    if (len = radio.getDynamicPayloadSize()) {
      client.startRoutedFrame(pipe_num, 0);
      // Fetch the payload
      radio.read( payload, len );
      for (uint8_t i = 0; i < len; i++) {
        client.addToPayload(payload[i]);
      }
      client.writeToBuffer();
      rx_blink = 1;

    } else{
      rx_error++;
      radio.read( payload, len );
    }
    if(count>10) return;
  } 
  delay(1);
}



