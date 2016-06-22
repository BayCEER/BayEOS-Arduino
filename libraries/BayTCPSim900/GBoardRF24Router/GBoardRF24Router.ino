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
#define BOARD GBoard
//#define BOARD GBoardPro
#define SKETCH_DEBUG 0

#define NRF24_CHANNEL 0x51
//#define NRF24_2CHANNEL 0x52


#define RX_LED A5
#define TX_LED A4
#define POWER_AD_PIN A3
#define DIVIDER (470.0+100.0)/100.0
/*
#define RX_LED 0
#define TX_LED 1
#define POWER_AD_PIN A0
#define DIVIDER (470.0+68.0)/68.0
*/
//

/*
  #define RX_LED A15
  #define TX_LED A14
  #define POWER_AD_PIN A12
*/
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
#define MAX_BUFFER_AVAILABLE 5000

unsigned long next_alive, next_send, next_try;
uint16_t rx_ok, rx_error, tx_error, rx1_count, rx2_count;
uint8_t tx_res;


//BayDebug client;
BayEOSBufferSDFat myBuffer;
extern volatile unsigned long timer0_millis;

volatile uint16_t wdcount = 0;
volatile uint8_t wdreset = 0;
volatile uint8_t tx_blink = 0;
volatile uint8_t rx_blink = 0;
volatile uint8_t rx_on = 0;
volatile uint8_t tx_on = 0;
volatile uint8_t program_pos = 0;
//volatile uint16_t wdt_millis;
//volatile uint16_t last_wdt_millis;
uint16_t wdt_sleep_time, current_wdcount;
//uint16_t sleep_time;
uint32_t total_sleep_time;
ISR(WDT_vect) {
  wdcount++;
  //  last_wdt_millis = wdt_millis;
  //  wdt_millis = millis();
  if (wdreset) {
    wdcount = 0;
    wdreset = 0;
  }
  if (wdcount > 480) { //no action for more than 120 sec.
#if SKETCH_DEBUG
    Serial.println("RESET");
    delay(100);
#endif
    if (program_pos > 0 && program_pos < 20)
      EEPROM.write(0, program_pos);
    asm volatile (" jmp 0"); //restart programm
  }
#ifdef TX_LED
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
#endif
#ifdef RX_LED
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
#endif
}



void initRF24(void) {
  radio.powerUp();
  radio.setChannel(NRF24_CHANNEL);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  //  radio.openWritingPipe(pipe_0);
  radio.openReadingPipe(0, pipe_0);
  radio.openReadingPipe(1, pipe_1);
  radio.openReadingPipe(2, pipe_2);
  radio.openReadingPipe(3, pipe_3);
  radio.openReadingPipe(4, pipe_4);
  radio.openReadingPipe(5, pipe_5);

  radio.startListening();
#ifdef NRF24_2CHANNEL
  radio2.powerUp();
  radio2.setChannel(NRF24_2CHANNEL);
  radio2.setPayloadSize(32);
  radio2.enableDynamicPayloads();
  radio2.setCRCLength( RF24_CRC_16 ) ;
  radio2.setDataRate(RF24_250KBPS);
  radio2.setPALevel(RF24_PA_MAX);
  radio2.setRetries(15, 15);
  radio2.setAutoAck(true);
  //  radio.openWritingPipe(pipe_0);
  radio2.openReadingPipe(0, pipe_0);
  radio2.openReadingPipe(1, pipe_1);
  radio2.openReadingPipe(2, pipe_2);
  radio2.openReadingPipe(3, pipe_3);
  radio2.openReadingPipe(4, pipe_4);
  radio2.openReadingPipe(5, pipe_5);
  radio2.startListening();
#endif
}

uint8_t handleRF24(void) {
  program_pos = 1;
  uint8_t pipe_num, len;
  uint8_t payload[32];
  uint8_t count;
  uint8_t rx=0;
  while (radio.available(&pipe_num)) {
    wdreset = 1;
    count++;
    if (len = radio.getDynamicPayloadSize()) {
      rx++;
      client.startRoutedFrame(pipe_num, 0);
      // Fetch the payload
      if (len > 32) len = 32;
      radio.read( payload, len );
      for (uint8_t i = 0; i < len; i++) {
        client.addToPayload(payload[i]);
      }
      if ((client.getPayload(5) == BayEOS_DataFrame) || (client.getPayload(5) == BayEOS_DelayedFrame)) {
        client.writeToBuffer();
        rx_blink = 1;
        rx1_count++;
      } else
        rx_error++;

    } else {
      rx_error++;
      radio.read( payload, len );
    }
    if (count > 10) break;
  }

#ifdef NRF24_2CHANNEL
  count=0;
  while (radio2.available(&pipe_num)) {
    wdreset = 1;
    count++;
    if (len = radio2.getDynamicPayloadSize()) {
      rx++;
      client.startRoutedFrame(pipe_num, 1);
      // Fetch the payload
      if (len > 32) len = 32;
      radio2.read( payload, len );
      for (uint8_t i = 0; i < len; i++) {
        client.addToPayload(payload[i]);
      }
      if ((client.getPayload(5) == BayEOS_DataFrame) || (client.getPayload(5) == BayEOS_DelayedFrame)) {
        client.writeToBuffer();
        rx_blink = 1;
        rx2_count++;
      } else
        rx_error++;


    } else {
      rx_error++;
      radio2.read( payload, len );
    }
    if (count > 10) break;
  }
  program_pos = 2;

#endif
  if(rx_error>5) initRF24();
  
  delay(1);
  return rx;
}



void startGPRS(uint8_t checkRF24=1) {
  program_pos = 3;
  uint8_t try_count = 0;
  while (try_count < 5) {
    wdreset = 1;
    if(checkRF24)
      handleRF24();
    try_count++;
    rx_blink = 1;
    tx_res = client.begin(38400);
    tx_blink = tx_res + 1;
    delay(600*tx_res);
#if SKETCH_DEBUG
    Serial.print("Client.begin: ");
    Serial.println(tx_res);
#endif
    if (! tx_res) return;
    if (try_count % 2 == 0) client.softSwitch();
    program_pos = 4;

  }
  return;

}

void setup(void) {
  program_pos = 5;
#if SKETCH_DEBUG
  Serial.begin(9600);
  Serial.println("Starting");
#endif
  Sleep.setupWatchdog(WDTO_250MS); //250ms
#ifdef RX_LED
  pinMode(RX_LED, OUTPUT);
#endif
#ifdef TX_LED
  pinMode(TX_LED, OUTPUT);
#endif

  wdt_reset();
  wdcount = 0;
  next_send = millis(); // just use this variable for temporal storage!
  rx_blink = 3;
  delay(500);
  tx_blink = 2;
  delay(1500);
  noInterrupts();
  current_wdcount = wdcount;
  interrupts();
  while (wdcount == current_wdcount) delayMicroseconds(10);
  delayMicroseconds(100);
  wdt_sleep_time = (millis() - next_send) / wdcount;

  radio.begin();
#ifdef NRF24_2CHANNEL
  radio2.begin();
#endif

  while (!SD.begin(SD_CSPIN)) {
    rx_blink = 2;
    tx_blink = 2;
    delay(2000);
  }

  initRF24();
  client.readConfigFromFile("GPRS.TXT");  
  startGPRS(0);
  myBuffer = BayEOSBufferSDFat(2000000000L, 1); //Append mode!

#if (BOARD == GBoardPro)
  myRTC.adjust(client.now());
  myBuffer.setRTC(myRTC, 0); //Relative Mode...
#endif
  client.setBuffer(myBuffer, 0);

  client.startFrame(BayEOS_Message);
  client.addToPayload("FW ");
  client.addToPayload(__DATE__);
  program_pos = EEPROM.read(0);
  client.addToPayload(" E:");
  client.addToPayload('0' + program_pos);
  client.writeToBuffer();

#if SKETCH_DEBUG
  Serial.println("mark1");
#endif

  next_send = 0;
  next_alive = 0;
  next_try = 0;
  tx_error = 0;
#if SKETCH_DEBUG
  Serial.print("Millis: ");
  Serial.print(millis());
  Serial.print(" ");
  Serial.println(next_alive);
  //delay(1000);
  //analogReference(DEFAULT);
  //  for(uint8_t z=0;z<10;z++){
  //    Serial.println(analogRead(A3));
  //    delay(10);
  //  }
#endif
  program_pos = 6;

}

void loop(void) {
  wdreset = 1;
  program_pos = 7;

  if (millis() > next_alive) {
#if SKETCH_DEBUG
    Serial.print("Alive: ");
    Serial.print(millis());
    Serial.print(" ");
    Serial.println(next_alive);
#endif
    //    radio.startListening();
    if (rx1_count == 0
#ifdef NRF24_2CHANNEL
        || rx2_count == 0
#endif
       ) initRF24();
    next_alive = millis() + SENDING_INTERVAL;
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myBuffer.readPos());
    client.addChannelValue(client.getRSSI());
    analogReference(DEFAULT);
    for (uint8_t z = 0; z < 3; z++) {
      analogRead(POWER_AD_PIN);
      delay(10);
    }
    client.addChannelValue((float)analogRead(POWER_AD_PIN) / 1023 * 3.3 * DIVIDER);
    client.addChannelValue(total_sleep_time / 1000);
    client.addChannelValue(wdt_sleep_time);
    client.addChannelValue(tx_error);
    client.addChannelValue(tx_res);
    client.addChannelValue(rx1_count);
    rx1_count = 0;
#ifdef NRF24_2CHANNEL
    client.addChannelValue(rx2_count);
    rx2_count = 0;
#endif
    client.writeToBuffer();
    program_pos = 8;
  }


  if ((tx_error == 0 && (millis() > next_send || myBuffer.available() > MAX_BUFFER_AVAILABLE) )
      || (tx_error && millis() > next_try)) {
#if SKETCH_DEBUG
    Serial.print("Send: ");
    Serial.print(millis());
    Serial.print(" ");
    Serial.println(next_send);
#endif
    program_pos = 9;
    wdt_reset();
    next_send = millis() + SENDING_INTERVAL;
    next_try = millis() + NEXT_TRY_INTERVAL;
    wdcount = 0;

    if (tx_res = client.sendMultiFromBuffer() ) {
      tx_error++;
    } else {
      tx_error = 0;
#if (BOARD == GBoardPro)
      myRTC.adjust(client.now());
#endif
      noInterrupts();
      current_wdcount = wdcount;
      interrupts();
      while (current_wdcount < 5) {
        delay(100);
        noInterrupts();
        current_wdcount = wdcount;
        interrupts();
      }

      while (wdcount == current_wdcount) delayMicroseconds(10);
      delayMicroseconds(100);
      wdt_sleep_time = (millis() + SENDING_INTERVAL - next_send) / wdcount;
    }
    tx_blink = tx_res + 1;

#if SKETCH_DEBUG
    Serial.print("TX Error: ");
    Serial.print(millis());
    Serial.print(" ");
    Serial.println(tx_error);
#endif
    if (tx_error % 5 == 4) {
      client.softSwitch();
      client.startFrame(BayEOS_Message);
      client.addToPayload("TX-ERROR SoftSwitch");
      client.writeToBuffer();
      startGPRS();
      program_pos = 10;
    }
  }



  if (! handleRF24()) {
    /*
    program_pos = 11;
    wdt_reset();
    Sleep.sleep();
    noInterrupts();
    timer0_millis += wdt_sleep_time;
    interrupts();
    total_sleep_time += wdt_sleep_time;
    program_pos = 12;*/
    delay(100);
  }

}


