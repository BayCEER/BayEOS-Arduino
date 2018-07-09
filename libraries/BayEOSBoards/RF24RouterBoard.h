/*
 * This is a header file, designed to make scetches of Low Current Board easier
 *
 * It depends on some conventions:
 * 1. bayeos.client must be named "client"
 * 2. Dallas is called "ds"
 * 3. Changes in CONSTANTS must be declared before include
 */

#include <Sleep.h>
#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <RF24.h>
RF24 radio(9, 10);
uint16_t rx1_count, rx1_error;

#ifdef NRF24_2CHANNEL
RF24 radio2(A0, A1);
uint16_t rx2_count, rx2_error;
#endif

#ifdef NRF24_3CHANNEL
RF24 radio3(A2, A3);
uint16_t rx3_count, rx3_error;
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
#define LED_RX1 4
#define LED_RX2 3
#define LED_RX3 2
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
#include <BayTCPESP8266.h>
BayESP8266 client(Serial, POWER_PIN); //No Power Pin
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
#ifdef NRF24_2CHANNEL
	if((led_blink_rx2>0) && ((ticks%LED_TICK_DIV)==0)) {
		if(digitalRead(LED_RX2)) led_blink_rx2--;
		digitalWrite(LED_RX2,!digitalRead(LED_RX2));
	}
#endif
#ifdef NRF24_3CHANNEL
	if((led_blink_rx3>0) && ((ticks%LED_TICK_DIV)==0)) {
		if(digitalRead(LED_RX3)) led_blink_rx3--;
		digitalWrite(LED_RX3,!digitalRead(LED_RX3));
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
	uint16_t end_ticks = ticks + _millis + 1;
	interrupts();
	do {
		sleepLCB();
	} while ((ticks < end_ticks));
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

	pinMode(LED_PIN, OUTPUT);
	pinMode(LED_RX1, OUTPUT);
#ifdef NRF24_3CHANNEL
	pinMode(LED_RX2, OUTPUT);
#endif
#ifdef NRF24_3CHANNEL
	pinMode(LED_RX3, OUTPUT);
#endif

  myBuffer.init(flash); //This will restore old pointers
  myBuffer.skip();
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  //We could also try to use absolute times received from GPRS!
  client.setBuffer(myBuffer);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
#ifdef GPRS_CONFIG
  client.readConfigFromStringPGM(PSTR(GPRS_CONFIG));
#else
  client.readConfigFromStringPGM(PSTR(WLAN_CONFIG));
#endif

  blinkLED(2);
  adjust_OSCCAL();
  tx_res = client.begin(38400);
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

	radio.startListening();
#ifdef NRF24_2CHANNEL
	radio2.begin();
	radio2.powerUp();
	radio2.setChannel(NRF24_2CHANNEL);
	radio2.setPayloadSize(32);
	radio2.enableDynamicPayloads();
	radio2.setCRCLength( RF24_CRC_16 );
	radio2.setDataRate(RF24_250KBPS);
	radio2.setPALevel(RF24_PA_MAX);
	radio2.setRetries(15, 15);
	radio2.setAutoAck(true);
	radio2.openReadingPipe(0, pipe_0);
	radio2.openReadingPipe(1, pipe_1);
	radio2.openReadingPipe(2, pipe_2);
	radio2.openReadingPipe(3, pipe_3);
	radio2.openReadingPipe(4, pipe_4);
	radio2.openReadingPipe(5, pipe_5);
	radio2.startListening();
#endif
#ifdef NRF24_3CHANNEL
	radio3.begin();
	radio3.powerUp();
	radio3.setChannel(NRF24_3CHANNEL);
	radio3.setPayloadSize(32);
	radio3.enableDynamicPayloads();
	radio3.setCRCLength( RF24_CRC_16 );
	radio3.setDataRate(RF24_250KBPS);
	radio3.setPALevel(RF24_PA_MAX);
	radio3.setRetries(15, 15);
	radio3.setAutoAck(true);
	radio3.openReadingPipe(0, pipe_0);
	radio3.openReadingPipe(1, pipe_1);
	radio3.openReadingPipe(2, pipe_2);
	radio3.openReadingPipe(3, pipe_3);
	radio3.openReadingPipe(4, pipe_4);
	radio3.openReadingPipe(5, pipe_5);
	radio3.startListening();
#endif
}

uint8_t handleRF24(void) {
	uint8_t pipe_num, len;
	uint8_t payload[32];
	char origin[] = "A0";
#ifdef RF24_P1_LETTER
	origin[0]=RF24_P1_LETTER;
#endif
	uint8_t count;
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

#ifdef NRF24_2CHANNEL
	count=0;
#ifdef RF24_P2_LETTER
	origin[0]=RF24_P2_LETTER;
#else
	origin[0]='B';
#endif

	while (radio2.available(&pipe_num)) {
		count++;
		if (len = radio2.getDynamicPayloadSize()) {
			rx++;
			origin[1]='0'+pipe_num;
			client.startOriginFrame(origin,1); //Routed Origin!
			// Fetch the payload
			if (len > 32) len = 32;
			radio2.read( payload, len );
			for (uint8_t i = 0; i < len; i++) {
				client.addToPayload(payload[i]);
			}
#if WITH_RF24_CHECKSUM
			if(! client.validateChecksum()) {
				client.writeToBuffer();
				led_blink_rx2 = 1;
				rx2_count++;
			} else
			rx2_error++;
#else
			client.writeToBuffer();
			led_blink_rx2 = 1;
			rx2_count++;
#endif

		} else {
			rx2_error++;
			radio2.read( payload, len );
		}
		if (count > 10) break;
	}

#endif

#ifdef NRF24_3CHANNEL
	count=0;
#ifdef RF24_P3_LETTER
	origin[0]=RF24_P3_LETTER;
#else
	origin[0]='C';
#endif

	while (radio3.available(&pipe_num)) {
		count++;
		if (len = radio3.getDynamicPayloadSize()) {
			rx++;
			origin[1]='0'+pipe_num;
			client.startOriginFrame(origin,1); //Routed Origin!
			// Fetch the payload
			if (len > 32) len = 32;
			radio3.read( payload, len );
			for (uint8_t i = 0; i < len; i++) {
				client.addToPayload(payload[i]);
			}
#if WITH_RF24_CHECKSUM
			if(! client.validateChecksum()) {
				client.writeToBuffer();
				led_blink_rx3 = 1;
				rx3_count++;
			} else
			rx3_error++;
#else
			client.writeToBuffer();
			led_blink_rx3 = 1;
			rx3_count++;
#endif

		} else {
			rx3_error++;
			radio3.read( payload, len );
		}
		if (count > 10) break;
	}

#endif
	if (count > 10)
		initRF24();

	delay(1);
	return rx;
}


void checkAction0(void){
	if(! ISSET_ACTION(0)) return;
    UNSET_ACTION(0);
    digitalWrite(POWER_PIN, HIGH);
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
#ifdef WLAN_CONFIG
    client.addChannelValue(0);
#else
    if (gprs_status)
      client.addChannelValue(client.getRSSI());
    else
      client.addChannelValue(0);
#endif
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
#ifdef NRF24_3CHANNEL
    client.addChannelValue(rx3_count);
    client.addChannelValue(rx3_error);
    rx3_count = 0;
    rx3_error = 0;
#endif
    client.writeToBuffer();

    if (! gprs_status && batLCB > 3.9) {
      client.begin(38400);
      initRF24();
      gprs_status = 1;
    }
    if (batLCB < 3.7) {
      if (gprs_status) {
        radio.powerDown();
#ifdef NRF24_2CHANNEL
        radio2.powerDown();
#endif
#ifdef NRF24_3CHANNEL
        radio3.powerDown();
#endif
       gprs_status = 0;
      }
    }

    if (gprs_status) {
      tx_res = client.sendMultiFromBuffer(1000);
      blinkLED(tx_res + 1);

      while (! tx_res && myBuffer.available() && ! ISSET_ACTION(0)) {
        handleRF24();
        tx_res = client.sendMultiFromBuffer(1000);
        blinkLED(tx_res + 1);
      }
    } else
      digitalWrite(POWER_PIN, LOW);
#ifdef WLAN_CONFIG
    client.powerDown();
#endif


}
