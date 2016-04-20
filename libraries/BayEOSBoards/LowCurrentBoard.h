/*
 * This is a header file, designed to make scetches of Low Current Board easier
 *
 * It depends on some conventions:
 * 1. bayeos.client must be named "client"
 * 2. Dallas is called "ds"
 * 3. Changes in CONSTANTS must be declared before include
 */

#ifndef LCB_BAT_MULTIPLIER
#define LCB_BAT_MULTIPLIER 1.1*320/100/1023
#endif

#ifndef TICKS_PER_SECOND
#define TICKS_PER_SECOND 16
#endif

#ifndef SAMPLING_INT
#define SAMPLING_INT 16
#endif

#ifndef LED_PIN
#define LED_PIN 5
#endif

#ifndef POWER_PIN
#define POWER_PIN 7
#endif

#define ISSET_ACTION(nr) (1<<nr)&action
#define UNSET_ACTION(nr) action&= ~(1<<nr)

#ifndef ACTION_COUNT
#define ACTION_COUNT 7
#endif

#if TICKS_PER_SECOND>3
#define LED_TICK_DIV (TICKS_PER_SECOND/4)
#else
#define LED_TICK_DIV 1
#endif

volatile uint16_t ticks;
volatile uint8_t action;
volatile uint8_t seconds;
volatile uint8_t led_blink = 0;
volatile uint8_t led_on = 0;
uint8_t startup = 10;
float batLCB;
RTC_Timer2 myRTC;

/*
 * ISR for timer2
 * increments RTC-Time
 * sets action bits
 */
ISR(TIMER2_OVF_vect) {
	ticks++;
	if((ticks % TICKS_PER_SECOND)==0) {
//    myRTC._seconds += 1; //this is not atomic - so better do in loop!
		seconds++;
		uint16_t tick_mod=(ticks/TICKS_PER_SECOND)%SAMPLING_INT;
		if(tick_mod<ACTION_COUNT) {
			action|=(1<<tick_mod);
		} else {
			action|=(1<<7);
		}
	}

	if((led_blink>0) && ((ticks%LED_TICK_DIV)==0)) {
		digitalWrite(LED_PIN,led_on==0);
		led_on=(led_on==0);
		led_blink--;
	}

}
inline void handleRtcLCB(void) {
	if (seconds) {
		myRTC._seconds += seconds;
		seconds = 0;
	}
}

#if WITHRAINGAUGE
float rain_count=0;
volatile uint8_t rain_event=0;
volatile uint16_t rain_event_ticks;
void rain_isr(void) {
	rain_event=1;
	rain_event_ticks=ticks;
}
#ifndef RAINGAUGE_LAGTICKS
#define RAINGAUGE_LAGTICKS 12
#endif

void handleRainEventLCB(void) {
	if(rain_event) {
		detachInterrupt(0);
	}
	if(rain_event && ((ticks-rain_event_ticks)>RAINGAUGE_LAGTICKS)) {
		attachInterrupt(0,rain_isr,FALLING);
		rain_count++;
		rain_event=0;
	}
}
#endif

#if WITHWIND
volatile uint16_t wind_count=0;
volatile uint8_t wind_event=0;
volatile uint16_t wind_event_ticks;
volatile uint16_t min_wind_ticks=65535;
long windn=0;
long windo=0;
uint16_t wind_direction_count=0;
void wind_isr(void) {
	if((ticks-wind_event_ticks)>4) {
		if((ticks-wind_event_ticks)<min_wind_ticks)
		min_wind_ticks=ticks-wind_event_ticks;
		wind_count++;
		wind_event=1;
		wind_event_ticks=ticks;
	}
}

#ifndef WIND_DIRECTION_PIN
#define WIND_DIRECTION_PIN A2
#endif
#ifndef WIND_POWER_PIN
#define WIND_POWER_PIN A3
#endif

void readWindDirectionLCB() {
	wind_direction_count++;
	pinMode(WIND_POWER_PIN,OUTPUT);
	digitalWrite(WIND_POWER_PIN,HIGH);
	int adc=analogRead(WIND_DIRECTION_PIN);
	digitalWrite(WIND_POWER_PIN,LOW);
	pinMode(WIND_POWER_PIN,INPUT);

	if(adc<552) {
		windn+=7071;
		windo+=7071;
		return;
	}
	if(adc<602) {
		windo+=7071;
		windn+=-7071;
		return;
	}
	if(adc<683) {
		windo+=10000;
		return;
	}
	if(adc<761) {
		windn+=7071;
		windo+=-7071;
		return;
	}
	if(adc<806) {
		windn+=10000;
		return;
	}
	if(adc<857) {
		windn+=-7071;
		windo+=-7071;
		return;
	}
	if(adc<916) {
		windn+=-10000;
		return;
	}
	windo+=-10000;
	return;
}

#endif

#if WITHDALLAS
uint8_t channel;
const byte* new_addr;
#ifndef DALLAS_PIN
#define DALLAS_PIN 6
#endif

#ifndef DALLAS_CHANNELS
#define DALLAS_CHANNELS 4
#endif

#ifndef DALLAS_OFFSET
#define DALLAS_OFFSET 20
#endif

DS18B20 ds=DS18B20(DALLAS_PIN,DALLAS_OFFSET,DALLAS_CHANNELS); //Allow four sensors on the bus - channel 11-14

void readAndSendDallasLCB(void) {
	float temp;
	client.startDataFrame(BayEOS_ChannelFloat32le);
	while(channel=ds.getNextChannel()) {
		if(! ds.readChannel(channel,&temp)) {
			client.addChannelValue(temp,channel);
		}
	}
	client.sendOrBuffer();
}

#endif

/*
 * blinkLED is done by timer2
 */
void blinkLED(uint8_t times) {
	if (LED_PIN > -1)
		led_blink = 2 * times;
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

#if LED_PIN>-1
	pinMode(LED_PIN, OUTPUT);
#endif
#if WITHRAINGAUGE
	digitalWrite(2,HIGH); //Enable Pullup on Pin 2 == INT0
	attachInterrupt(0,rain_isr,FALLING);
	rain_count=0;
	rain_event=0;
#endif

#if WITHWIND
	attachInterrupt(1,wind_isr,RISING);
	wind_count=0;
	wind_event=0;
	windn=0;
	windo=0;
#endif

#if WITHDALLAS
	ds.setAllAddrFromEEPROM();
	// Search and Delete
	while(channel=ds.checkSensors()) {
		new_addr=ds.getChannelAddress(channel);
		client.startFrame((uint8_t) BayEOS_Message);
		client.addToPayload(String("DS:")+channel+"-"+ds.addr2String(new_addr));
		client.writeToBuffer();
		ds.deleteChannel(new_addr);
	}
	while(new_addr=ds.search()) {
		if(channel=ds.getNextFreeChannel()) {
			ds.addSensor(new_addr,channel);
			client.startFrame((uint8_t) BayEOS_Message);
			client.addToPayload(String("DS:")+channel+"+"+ds.addr2String(new_addr));
			client.writeToBuffer();
		}
	}
#endif

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

void readBatLCB() {
	analogReference (INTERNAL);
	pinMode(POWER_PIN, OUTPUT);
	digitalWrite(POWER_PIN, HIGH);
	batLCB = LCB_BAT_MULTIPLIER * analogRead(A0);
	digitalWrite(POWER_PIN, LOW);
	pinMode(POWER_PIN, INPUT);
	analogReference (DEFAULT);
}

void startLCB() {
	blinkLED(3);
	delayLCB(2000);
	noInterrupts();
	ticks = SAMPLING_INT * TICKS_PER_SECOND - 1;
	interrupts();
	action = 0;
}

void sendOrBufferLCB() {
	if (startup) {
		if (client.sendPayload()) {
			client.writeToBuffer();
			blinkLED(2);
		} else
			blinkLED(1);
		startup--;
	} else {
		client.sendOrBuffer();
	}

}
