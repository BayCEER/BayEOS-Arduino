/*
 * Functions and Variables for RF24Routing
 *
 * expects the following global variables to be defined:
 * radio, client
 */
uint16_t rx_ok, rx_error, rx1_count, rx2_count;

#ifndef WITH_RF24_CHECKSUM
#define WITH_RF24_CHECKSUM 0
#endif

volatile uint16_t wdcount = 0;
volatile uint8_t wdreset = 0;
volatile uint8_t tx_blink = 0;
volatile uint8_t rx_blink = 0;

#ifndef POWER_DIVIDER
#define POWER_DIVIDER (470.0+100.0)/100.0
#endif

#ifndef POWER_AD_PIN
#define POWER_AD_PIN A3
#endif

#ifndef SENDING_INTERVAL
#define SENDING_INTERVAL 180000L
#endif

#ifndef NEXT_TRY_INTERVAL
#define NEXT_TRY_INTERVAL 300000L
#endif

#ifndef MAX_BUFFER_AVAILABLE
#define MAX_BUFFER_AVAILABLE 5000
#endif

unsigned long last_alive, last_send, last_try;
uint16_t tx_error;
uint8_t tx_res;

ISR(WDT_vect) {
	wdcount++;
	//  last_wdt_millis = wdt_millis;
	//  wdt_millis = millis();
	if (wdreset) {
		wdcount = 0;
		wdreset = 0;
	}
	if (wdcount > 960) { //no action for more than 240 sec.
#if SKETCH_DEBUG
		Serial.println("RESET");
		delay(100);
#endif
		asm volatile (" jmp 0"); //restart programm
	}
#ifdef TX_LED
	if (tx_blink) {
		if (digitalRead(TX_LED)) {
			digitalWrite(TX_LED, LOW);
			tx_blink--;
		} else {
			digitalWrite(TX_LED, HIGH);
		}
	}
#endif
#ifdef RX_LED
	if (rx_blink) {
		if (digitalRead(RX_LED)) {
			digitalWrite(RX_LED, LOW);
			rx_blink--;
		} else {
			digitalWrite(RX_LED, HIGH);
		}
	}
#endif
}

void initWatchdog(void) {
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
#ifdef RX_LED
	rx_blink = 2;
	delay(500);
	tx_blink = 1;
	delay(1000);
#endif
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
	//  radio.openWritingPipe(pipe_0);
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
	uint8_t pipe_num, len;
	uint8_t payload[32];
	char origin[] = "P0";
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
				rx_blink = 1;
				rx1_count++;
			} else
			rx_error++;
#else
			client.writeToBuffer();
			rx_blink = 1;
			rx1_count++;
#endif

		} else {
			rx_error++;
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
	origin[0]='R';
#endif
		
	while (radio2.available(&pipe_num)) {
		wdreset = 1;
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
				rx_blink = 1;
				rx2_count++;
			} else
			rx_error++;
#else
			client.writeToBuffer();
			rx_blink = 1;
			rx2_count++;
#endif

		} else {
			rx_error++;
			radio2.read( payload, len );
		}
		if (count > 10) break;
	}

#endif
	if (rx_error > 5)
		initRF24();
	if (count > 10)
		initRF24();

	delay(1);
	return rx;
}

void startGPRS(uint8_t checkRF24 = 1) {
	uint8_t try_count = 0;
	while (try_count < 5) {
		wdreset = 1;
		if (checkRF24)
			handleRF24();
		try_count++;
		rx_blink = 1;
		tx_res = client.begin(38400);
		tx_blink = tx_res + 1;
		delay(600 * tx_res);
#if SKETCH_DEBUG
		Serial.print("Client.begin: ");
		Serial.println(tx_res);
#endif
		if (!tx_res)
			return;
		if (try_count % 2 == 0)
			client.softSwitch();
	}
	return;

}

void checkAlive(void) {
	if ((millis() - last_alive) < SENDING_INTERVAL)
		return;
#if SKETCH_DEBUG
	Serial.print("Alive: ");
	Serial.print(millis());
	Serial.print(" ");
	Serial.println(last_alive);
#endif
	//    radio.startListening();
	if (rx1_count == 0
#ifdef NRF24_2CHANNEL
			|| rx2_count == 0
#endif
			)
		initRF24();
	last_alive = millis();
	client.startDataFrame(BayEOS_Float32le);
	client.addChannelValue(millis() / 1000);
	client.addChannelValue(myBuffer.writePos());
	client.addChannelValue(myBuffer.readPos());
	client.addChannelValue(client.getRSSI());
	analogReference (DEFAULT);
	for (uint8_t z = 0; z < 3; z++) {
		analogRead (POWER_AD_PIN);
		delay(10);
	}
	client.addChannelValue(
			(float) analogRead(POWER_AD_PIN) / 1023 * 3.3 * POWER_DIVIDER);
	client.addChannelValue(tx_error);
	client.addChannelValue(tx_res);
	client.addChannelValue(rx1_count);
	rx1_count = 0;
#ifdef NRF24_2CHANNEL
	client.addChannelValue(rx2_count);
	rx2_count = 0;
#endif
	client.writeToBuffer();

}

void checkSend(void) {
	if (!myBuffer.available())
		return;
	if ((millis() - last_send) < SENDING_INTERVAL
			&& myBuffer.available() < MAX_BUFFER_AVAILABLE)
		return;
	if (tx_error && (millis() - last_try) < NEXT_TRY_INTERVAL)
		return;

#if SKETCH_DEBUG
	Serial.print("Send: ");
	Serial.print(millis());
	Serial.print(" ");
	Serial.println(last_send);
#endif
	last_send = millis();
	last_try = millis();

	if (tx_res = client.sendMultiFromBuffer(MAX_BUFFER_AVAILABLE)) {
		tx_error++;
	} else {
		tx_error = 0;
#if (BOARD == GBoardPro)
		myRTC.adjust(client.now());
#endif
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
	}

}
