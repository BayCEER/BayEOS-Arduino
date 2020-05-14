/*
 * This is a header file, designed to make scetches of Low Current Board easier
 *
 * It depends on some conventions:
 * 1. bayeos.client must be named "client"
 * 2. Changes in CONSTANTS must be declared before include
 */

#include <Sleep.h>
#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
BaySerial rx_client=BaySerial(Serial);
uint16_t rx1_count, rx1_error;
#define CTS_PIN 6
#define TXRX_PIN 4

#ifdef NRF24_CHANNEL
#include <RF24.h>
RF24 radio(9, 10);
uint16_t rx2_count, rx2_error;

#endif


uint8_t tx_res, tx_error;
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
uint8_t gprs_status; //1 = 0n, 0 = Off

#ifndef BAT_MULTIPLIER
#define BAT_MULTIPLIER 3.3*200/100/1023
#endif

#ifndef TICKS_PER_SECOND
#define TICKS_PER_SECOND 16
#endif

#ifndef SAMPLING_INT
#define SAMPLING_INT 128
#endif

#ifndef BAUD_RATE
#define BAUD_RATE 38400
#endif

#define LED_PIN 5
#define LED_RX1 3
#define LED_RX2 2
#define POWER_PIN 7

volatile uint16_t ticks;
volatile uint8_t action;


#define ISSET_ACTION(nr) ((1<<nr)&action)
#define UNSET_ACTION(nr) (action&= ~(1<<nr))

#ifndef ACTION_COUNT
#define ACTION_COUNT 1
#endif



#ifndef CHECKSUM_FRAMES
#define CHECKSUM_FRAMES 0
#endif

#ifndef RESET_COUNT
#define RESET_COUNT 8
#endif


#if TICKS_PER_SECOND>3
#define LED_TICK_DIV (TICKS_PER_SECOND/4)
#else
#define LED_TICK_DIV 1
#endif


#if RESET_COUNT
volatile uint8_t action0_pending_count=0;
#endif

volatile uint8_t seconds;
volatile unsigned long current_micros, last_micros;
volatile uint8_t adjust_osccal_flag;
volatile uint8_t led_blink = 0;
volatile uint8_t led_blink_rx1 = 0;
volatile uint8_t led_blink_rx2 = 0;
volatile uint8_t led_blink_rx3 = 0;

#ifndef SENDING_INTERVAL
#define SENDING_INTERVAL 120000L
#endif

#ifndef NEXT_TRY_INTERVAL
#define NEXT_TRY_INTERVAL 300000L
#endif

#ifndef MAX_BUFFER_AVAILABLE
#define MAX_BUFFER_AVAILABLE 2000
#endif

float batLCB;
RTC_Timer2 myRTC;

#ifdef RTC_SECOND_CORRECT
volatile long rtc_seconds_correct;
#endif

#ifdef GPRS_CONFIG
#include <BayTCPSim900.h>
BayGPRS client(Serial, 0); //No Power Pin
#else
#ifdef WLAN_CONFIG
#include <BayTCPESP8266.h>
BayESP8266 client(Serial, POWER_PIN);
#else
#include <BayDebug.h>
BayDebug client(Serial);
#define DEBUG_CONFIG 1
#endif
#endif


/*
 * ISR for timer2
 * increments RTC-Time
 * sets action bits
 */
ISR(TIMER2_OVF_vect) {
	ticks++;
	if(adjust_osccal_flag){
		last_micros=current_micros;
		current_micros=micros();
		adjust_osccal_flag++;
		if(adjust_osccal_flag>2){
			if((current_micros-last_micros)>(1005000L/TICKS_PER_SECOND)){
				if(OSCCAL) OSCCAL--;
				else adjust_osccal_flag=0; //reached limit!
			}
			else if((current_micros-last_micros)<(995000L/TICKS_PER_SECOND)){
				if(OSCCAL<255) OSCCAL++;
				else adjust_osccal_flag=0; //reached limit!
			}
			else {
				//Timing is ok :-)
				adjust_osccal_flag=0;
			}

		}

	}

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
#ifdef NRF24_CHANNEL
	if((led_blink_rx2>0) && ((ticks%LED_TICK_DIV)==0)) {
		if(digitalRead(LED_RX2)) led_blink_rx2--;
		digitalWrite(LED_RX2,!digitalRead(LED_RX2));
	}
#endif
}

/*
 * Adjust the OSCCAL of internal oscillator using TIMER2 RTC
 */
void adjust_OSCCAL(void){
	adjust_osccal_flag=1;
	while(adjust_osccal_flag){
		delay(1);
	}
}



/*
 * blinkLED is done by timer2
 */
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

/*
 * init Time2 and LED pin
 */
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

	pinMode(TXRX_PIN, OUTPUT);
	digitalWrite(TXRX_PIN,0); //TX-MODE
	pinMode(CTS_PIN,OUTPUT);
	digitalWrite(CTS_PIN,0); //Block RX

	pinMode(LED_PIN, OUTPUT);
	pinMode(LED_RX1, OUTPUT);
#ifdef NRF24_CHANNEL
	pinMode(LED_RX2, OUTPUT);
#endif

  myBuffer.init(flash); //This will restore old pointers
  myBuffer.skip();
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  //We could also try to use absolute times received from GPRS!
  client.setBuffer(myBuffer);
  rx_client.setBuffer(myBuffer);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  blinkLED(2);
  adjust_OSCCAL();
#ifdef GPRS_CONFIG
  client.readConfigFromStringPGM(PSTR(GPRS_CONFIG));
  tx_res = client.begin(38400);
#else
#ifdef WLAN_CONFIG
  client.readConfigFromStringPGM(PSTR(WLAN_CONFIG));
  tx_res = client.begin(38400);
#else
  client.begin(38400,1);
#endif
#endif

#ifdef GPRS_CONFIG
  if (! tx_res) myRTC.adjust(client.now());
#endif
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

  tx_res = client.sendMessage("Router started");
  blinkLED(tx_res + 1);
  delay(1000 + tx_res * 500);

  /*
     1 == OK
     2 == no success
     3 == timeout
     4 == network timeout
     5 == gprs modem timeout
  */
//  if (myRTC.now().get() < 2000) myBuffer.skip(); //got no time! skip the unread frames in Buffer!!
  gprs_status = 1;
}
#ifdef NRF24_CHANNEL

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
	radio.openReadingPipe(0, pipe_0);
	radio.openReadingPipe(1, pipe_1);
	radio.openReadingPipe(2, pipe_2);
	radio.openReadingPipe(3, pipe_3);
	radio.openReadingPipe(4, pipe_4);
	radio.openReadingPipe(5, pipe_5);

}

uint8_t handleRF24(void) {
	uint8_t pipe_num, len;
	uint8_t payload[32];
	char origin[] = "A0";
#ifdef RF24_P1_LETTER
	origin[0]=RF24_P1_LETTER;
#endif
	uint8_t count=0;
	uint8_t rx = 0;
	while (radio.available(&pipe_num)) {
		count++;
		if (len = radio.getDynamicPayloadSize()) {
			rx++;
			origin[1] = '0' + pipe_num;
			client.startOriginFrame(origin, 1); //Routed Origin!
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
				rx2_count++;
			} else
			rx2_error++;
#else
			client.writeToBuffer();
			led_blink_rx1 = 1;
			rx2_count++;
#endif

		} else {
			rx2_error++;
		}
		if (count > 10)
			break;
	}
	if (count > 10)
		initRF24();

	delay(2);
	return rx;
}
#endif

void checkAction0(void){
	if(! ISSET_ACTION(0)) return;
    UNSET_ACTION(0);

    digitalWrite(POWER_PIN, HIGH);
	digitalWrite(TXRX_PIN,0); //TX-MODE
	pinMode(CTS_PIN,OUTPUT);
	digitalWrite(CTS_PIN,0); //Block RX
    analogReference (DEFAULT);
    if(! gprs_status) delayLCB(1000);
#ifdef WLAN_CONFIG
    delayLCB(100);
#endif
    adjust_OSCCAL();
    batLCB = BAT_MULTIPLIER * analogRead(A7);

    client.startDataFrame();
    client.addChannelValue(millis() / 1000);
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myBuffer.readPos());
#ifdef GPRS_CONFIG
    if (gprs_status)
      client.addChannelValue(client.getRSSI());
    else
      client.addChannelValue(0);
#else
    client.addChannelValue(0);
#endif
    client.addChannelValue(batLCB);
    client.addChannelValue(tx_error);
    client.addChannelValue(tx_res);
    client.addChannelValue(rx1_count);
    client.addChannelValue(rx1_error);
    rx1_count = 0;
    rx1_error = 0;
#ifdef NRF24_CHANNEL
    client.addChannelValue(rx2_count);
    client.addChannelValue(rx2_error);
    rx2_count = 0;
    rx2_error = 0;
#endif
    client.writeToBuffer();

#ifndef DEBUG_CONFIG
    if (! gprs_status && batLCB > 3.9) {
      client.begin(38400);
#ifdef NRF24_CHANNEL
      initRF24();
#endif
     gprs_status = 1;
    }
    if (batLCB < 3.7) {
      if (gprs_status) {
#ifdef NRF24_CHANNEL
        radio.powerDown();
#endif
       gprs_status = 0;
      }
    }
#endif

    if (gprs_status) {
#ifndef DEBUG_CONFIG
      tx_res = client.sendMultiFromBuffer(1000);
#else
      tx_res = client.sendFromBuffer();
#endif
      if(tx_res) tx_error++;
      else tx_error=0;
      blinkLED(tx_res + 1);
#ifndef DEBUG_CONFIG
      if(tx_error>5 && (tx_error % 5)==0){
    	    digitalWrite(POWER_PIN, LOW);
    	    delay(1000);
    	    digitalWrite(POWER_PIN, HIGH);
    	    client.begin(38400);
      }
#endif

      while (! tx_res && myBuffer.available() && ! ISSET_ACTION(0)) {
#ifdef NRF24_CHANNEL
        handleRF24();
#endif

#ifndef DEBUG_CONFIG
      tx_res = client.sendMultiFromBuffer(1000);
#else
      tx_res = client.sendFromBuffer();
#endif
        blinkLED(tx_res + 1);
      }
    } else {
      digitalWrite(POWER_PIN, LOW);
    }
#ifdef DEBUG_CONFIG
    Serial.flush();
#endif
    digitalWrite(TXRX_PIN,1); //RX-MODE
    pinMode(CTS_PIN,INPUT);//Free RX

#ifdef WLAN_CONFIG
    client.powerDown();
#endif


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

void readRX(){
 uint8_t res = rx_client.readIntoPayload();
 if (res == 0) {
   rx_client.writeToBuffer();
   led_blink_rx1=1;
   rx1_count++;
 } else if (res == 1) {
   rx1_error++;
 }
}
