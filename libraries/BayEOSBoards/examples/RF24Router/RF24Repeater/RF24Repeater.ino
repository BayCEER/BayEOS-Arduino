/*
 * Sketch for BayEOS-RF24-Repeater
 * 
 */
 
#define SAMPLING_INT 128
#define TICKS_PER_SECOND 16
#define MAX_SKIP (16*128)
#define NRF24_CHANNEL 0x2b
#define NRF24_TX_CHANNEL 0x5e
#define WITH_RF24_CHECKSUM 1
#define BAT_MULTIPLIER 3.3*200/100/1023
uint8_t pipe[]={0x12, 0xae, 0x31, 0xe4, 0x45};
const uint8_t pipes[] = {0x12, 0x24, 0x48, 0x96, 0xab, 0xbf};

#include <Sleep.h>
#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <RF24.h>
RF24 radio(9, 10);
uint16_t rx1_count, rx1_error, skip_counter;

#include <BayRF24.h>
BayRF24 client(A2,A3);
uint8_t tx_error, tx_error_count;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
uint8_t rf24_status; //1 = 0n, 0 = Off
#define LED_PIN 5
#define LED_RX1 4
#define LED_RX3 2
#define POWER_PIN 7

volatile uint16_t ticks;
volatile uint8_t action;


#define ISSET_ACTION(nr) ((1<<nr)&action)
#define UNSET_ACTION(nr) (action&= ~(1<<nr))

#define ACTION_COUNT 1
#define RESET_COUNT 8

#if TICKS_PER_SECOND>3
#define LED_TICK_DIV (TICKS_PER_SECOND/4)
#else
#define LED_TICK_DIV 1
#endif

#if RESET_COUNT
volatile uint8_t action0_pending_count=0;
#endif

volatile uint8_t seconds;
volatile uint8_t led_blink = 0;
volatile uint8_t led_blink_rx1 = 0;
volatile uint8_t led_blink_rx3 = 0;
float batLCB;
RTC_Timer2 myRTC;
#ifdef RTC_SECOND_CORRECT
volatile long rtc_seconds_correct;
#endif
ISR(TIMER2_OVF_vect) {
  ticks++;

  if((ticks % TICKS_PER_SECOND)==0) {
    myRTC._seconds ++; //RTC_Timer2.get() and adjust() are interrupt save now!
    seconds++;
#ifdef RTC_SECOND_CORRECT

#if RTC_SECOND_CORRECT < 0
    rtc_seconds_correct--;
    if(rtc_seconds_correct<=RTC_SECOND_CORRECT){
      rtc_seconds_correct=0;
      myRTC._seconds --;
      seconds--;
    }
#else
    rtc_seconds_correct++;
    if(rtc_seconds_correct>=RTC_SECOND_CORRECT){
      rtc_seconds_correct=0;
      myRTC._seconds ++;
      seconds++;
    }

#endif
#endif
    uint16_t tick_mod=myRTC._seconds%SAMPLING_INT;
    if(tick_mod<ACTION_COUNT) {
#if RESET_COUNT
      if(tick_mod==0){
        if(ISSET_ACTION(0))
          action0_pending_count++;

        else
          action0_pending_count=0;
        if(action0_pending_count>RESET_COUNT)
          asm volatile (" jmp 0"); //restart programm

      }
#endif
      action|=(1<<tick_mod);
    } else {
      action|=(1<<7);
    }
  }

  if((led_blink>0) && ((ticks%LED_TICK_DIV)==0)) {
    if(digitalRead(LED_PIN)) led_blink--;
    digitalWrite(LED_PIN,!digitalRead(LED_PIN));
  }
  
  if((led_blink_rx1>0) && ((ticks%LED_TICK_DIV)==0)) {
    if(digitalRead(LED_RX1)) led_blink_rx1--;
    digitalWrite(LED_RX1,!digitalRead(LED_RX1));
  }
  
  if((led_blink_rx3>0) && ((ticks%LED_TICK_DIV)==0)) {
    if(digitalRead(LED_RX3)) led_blink_rx3--;
    digitalWrite(LED_RX3,!digitalRead(LED_RX3));
  }

}

void blinkLED(uint8_t times) {
  if (LED_PIN > -1)
    led_blink = times;
}

void sleepLCB() {
  Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
}

void delayLCB(uint16_t _millis) {
  _millis /= (1000 / TICKS_PER_SECOND);
  noInterrupts();
  uint16_t end_ticks = ticks + _millis + 2;
  interrupts();
  do {
    sleepLCB();
  } while ((ticks != end_ticks));
}

void initLCB() {
#if TICKS_PER_SECOND==128
  Sleep.setupTimer2(1); //init timer2 to 0,0078125sec
#elif TICKS_PER_SECOND==16
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
#elif TICKS_PER_SECOND==4
      Sleep.setupTimer2(3); //init timer2 to 0,25sec
#elif TICKS_PER_SECOND==2
      Sleep.setupTimer2(4); //init timer2 to 0,5sec
#elif TICKS_PER_SECOND==1
      Sleep.setupTimer2(5); //init timer2 to 1sec
#else
#error unsupported TICKS_PER_SECOND
#endif

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_RX1, OUTPUT);
  pinMode(LED_RX3, OUTPUT);

  myBuffer.init(flash); //This will restore old pointers
  myBuffer.skip();
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  //We could also try to use absolute times received from GPRS!
  client.setBuffer(myBuffer);
  pinMode(POWER_PIN, OUTPUT);

  rf24_status = 1;
}


void startLCB() {
  blinkLED(3);
  delayLCB(2000);
  noInterrupts();
  action = 0;
  ticks = 0;
  myRTC._seconds=SAMPLING_INT-1;
  interrupts();
}

void initRF24(void) {
  radio.begin();
  radio.powerUp();
  radio.setChannel(NRF24_CHANNEL);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  for(uint8_t i=0;i<6;i++){
    pipe[0]=pipes[i];
    radio.openReadingPipe(i, pipe);
  }
  radio.startListening();
  client.init(pipe,NRF24_TX_CHANNEL);
}

uint8_t payload[40];

uint8_t handleRF24(void) {
  uint8_t pipe_num, len;
  uint8_t count=0;
  uint8_t rx = 0;
  while (radio.available(&pipe_num)) {
    count++;
    if (len = radio.getDynamicPayloadSize()) {
      rx++;
      client.startRF24Frame(pipe_num); //RF24-Frame
      if (len > 32)
        len = 32;
      radio.read(payload, len);
      for (uint8_t i = 0; i < len; i++) {
        client.addToPayload(payload[i]);
      }
#if WITH_RF24_CHECKSUM
      if(! client.validateChecksum()) {
        client.writeToBuffer();
        led_blink_rx1 = 1;
        rx1_count++;
      } else
      rx1_error++;
#else
      client.writeToBuffer();
      led_blink_rx1 = 1;
      rx1_count++;
#endif

    } else {
      rx1_error++;
      radio.read(payload, len);
    }
    if (count > 10)
      break;
  }

  if (count > 10)
    initRF24();

  delay(2);
  return rx;
}

void sendFromBuffer(void){
  if(! myBuffer.available()) return;
  if(tx_error && skip_counter<((uint16_t)tx_error_count*tx_error_count) && skip_counter<MAX_SKIP){
    skip_counter++;
    return;
  }
  skip_counter=0;
  myBuffer.initNextPacket();
  unsigned long p_delay=0;
  uint8_t pipe_nr=5;
  myBuffer.readPacket(payload); //copy packet to buffer
  if(payload[0]==BayEOS_RF24Frame){
    pipe_nr=payload[1];
    uint8_t offset=2;
    if(payload[2]==BayEOS_DelayedFrame){
      memcpy((void*)&p_delay,payload+3,4);
      offset+=5;
    }
    p_delay+=(myRTC.get()-myBuffer.packetMillis())*1000;
    client.startDelayedFrame(p_delay);
    while(offset<myBuffer.packetLength()){
      client.addToPayload(payload[offset]);
      offset++;
    }
  } else {
    client.readFromBuffer();
  }
  pipe[0]=pipes[pipe_nr];
  client.setTXAddr(pipe);
  if( client.sendPayload()){
    tx_error=1;
    if(tx_error_count<255) tx_error_count++;
    blinkLED(1);
  } else {
    tx_error=0;
    tx_error_count=0;
    led_blink_rx3 = 1;
    myBuffer.next();
  }
}


void checkAction0(void){
  if(! ISSET_ACTION(0)) return;
    UNSET_ACTION(0);
    digitalWrite(POWER_PIN, HIGH);
    analogReference (DEFAULT);
    batLCB = BAT_MULTIPLIER * analogRead(A7);
    digitalWrite(POWER_PIN, LOW);
    
    client.startDataFrameWithOrigin(BayEOS_Int16le,"RF24RP",1,WITH_RF24_CHECKSUM);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB*1000);
    client.addChannelValue(rx1_count);
#if WITH_RF24_CHECKSUM
    client.addChecksum();
#endif
    
    rx1_count = 0;
    rx1_error = 0;
    pipe[0]=pipes[5];
    client.setTXAddr(pipe);
    client.sendOrBuffer();

    if (! rf24_status && batLCB > 3.9) {
      initRF24();
      rf24_status = 1;
    }
    if (batLCB < 3.7) {
      if (rf24_status) {
        radio.powerDown();
        rf24_status = 0;
      }
    }
}

void setup()
{
  initLCB();
  initRF24();
  startLCB();
}

void loop()
{
  if (rf24_status)
    handleRF24();
  checkAction0();
  sendFromBuffer();
  sleepLCB();
}

