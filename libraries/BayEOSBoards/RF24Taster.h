#include <BayEOSCommands.h>
#include <BayEOSBufferRAM.h>
#include <BayRF24.h>
//#define RF24ADDRESS2 0x7616a24748LL /*via forward*/

#ifndef TASTER_VERSION
#define TASTER_VERSION 2
#endif

#ifndef WITHBUFFER
#define WITHBUFFER 1
#endif

#if TASTER_VERSION == 2
#define TOPLEFT 3
#define TOPRIGHT 2
#define BOTTOMLEFT 0
#define BOTTOMRIGHT 1
#else
#define TOPLEFT 1
#define TOPRIGHT 2
#define BOTTOMLEFT 0
#define BOTTOMRIGHT 3
#endif


BayRF24 client = BayRF24(9, 10);
unsigned long last_status;

#if WITHBUFFER
#ifdef __AVR_ATmega168P__
uint8_t buffer[100];
#else
uint8_t buffer[1000];
#endif

BayEOSBufferRAM myBuffer(buffer);
#endif


void longPressHandler(uint8_t nr);
void shortPressHandler(uint8_t nr);

#define SAMPLING_INT 120
#define TICKS_PER_SECOND 1
#define ACTION_COUNT 1
#include <LowCurrentBoard.h>

#if WITH_NTC
#include <NTC.h>
NTC_ADC ntc(A2, A1, 20000.0, 10.0);
#if WITH_HR202
#include <HR202.h>
HR202 hr(A5, A4, A3);
#endif
#endif

void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void sendCommand(uint8_t command, uint8_t channel = 0x01, uint8_t arg = 0) {
  client.startFrame(BayEOS_ChecksumFrame);
  client.addToPayload((uint8_t)BayEOS_Command);
  client.addToPayload((uint8_t)BayEOS_SwitchCommand);
  client.addToPayload(command);
  client.addToPayload(channel);
  client.addToPayload(arg);
  client.addChecksum();
#if BLINK_ON_SEND
  if (client.sendPayload()) blinkLED(2);
  else blinkLED(1);
#else
  client.sendPayload();
#endif

}


uint8_t pins[] = {2, 3, 6, 7};

volatile uint8_t int_flag;
void isr_int0(void) {
  if (! int_flag) int_flag = 0x1;
}

void isr_int1(void) {
  if (! int_flag) int_flag = 0x2;
}

ISR (PCINT2_vect) {
  if (int_flag) return;
  if (! digitalRead(6)) {
    int_flag = 0x3;
    return;
  }
  if (! digitalRead(7)) {
    int_flag = 0x4;
  }
}
uint16_t last_int;

void initBoard(void) {
  uint8_t temp = MCUCR;
  MCUCR = temp | (1 << IVCE);
  MCUCR = temp & ~(1 << IVSEL);
  initLCB();
  client.init(RF24ADDRESS, RF24CHANNEL);
#if WITHBUFFER
  myBuffer.setRTC(myRTC, 0); //relative Time
  client.setBuffer(myBuffer, 20);
#endif
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(pins[i], INPUT_PULLUP);
  }
  pciSetup(6);
  pciSetup(7);
  attachInterrupt(digitalPinToInterrupt(2), isr_int0, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), isr_int1, FALLING);
  startLCB();
  int_flag = 0;
}

uint8_t command = 0, channel = 0, int_pin;
uint8_t fast_ticks = 0;

void handlePushButton(void) {
  if (int_flag) {
    int_pin = (int_flag & 0xf) - 1;
    if (int_flag & 0x10) {
      if (! (int_flag & 0x20) && ! digitalRead(pins[int_pin]) && (ticks - last_int) > 6) {
        bitSet(int_flag, 5); //set bit 6 (long press)
        bitClear(int_flag, 6); //clear bit 7 (command send)
      }
      if (digitalRead(pins[int_pin]) && (ticks - last_int) > 1) {
        Sleep.setupTimer2(5); //1 ticks per s
        int_flag = 0; //clear flag
      }
    } else {
      last_int = ticks;
      Sleep.setupTimer2(2);//16 ticks/s
      bitSet(int_flag, 4); //set bit 5 (int registered)
    }
  }
  if (int_flag) {
    if (! (int_flag & 0x40)) {
      bitSet(int_flag, 6);
      // channel = (int_pin < 2 ? 1 : 2);
      uint8_t arg=0;
      if (int_flag & 0x20) { //long press
    	  longPressHandler(int_pin);
    	//(*handleLongPressCommand)(int_pin);
      } else { //short press
    	  shortPressHandler(int_pin);
      	//(*handleShortPressCommand)(int_pin);
      }
      client.setTXAddr(RF24ADDRESS);
      delay(300);

    }
  }
}

void sendStatus(void){
#if STATUS_SEND
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    client.startDataFrameWithOrigin(BayEOS_Int16le, STATUS_NAME, 1);
    pinMode(A0, OUTPUT);
    digitalWrite(A0, HIGH);
    analogReference(INTERNAL);
    int adc = analogRead(A7);
    digitalWrite(A0, LOW);
    pinMode(A0, INPUT);
    client.addChannelValue( 1.1 * 320 / 100 / 1023 * 1000 * adc);
    analogReference(DEFAULT);
    client.addChannelValue((float)(millis()-last_status)*10/SAMPLING_INT);
    last_status=millis();
#if WITH_NTC
    float t = ntc.getTemp();
    client.addChannelValue(100 * t);
#if WITH_HR202
   float h = hr.getHumidity(t);
   client.addChannelValue(100 * h);
#endif
#endif
    client.addChecksum();
#if WITHBUFFER
    client.sendOrBuffer();
#else
    client.sendPayload();
#endif
  }
#if WITHBUFFER
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
#endif
#endif
}
