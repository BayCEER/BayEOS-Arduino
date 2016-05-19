#ifndef WITH_TFT
#define WITH_TFT 0
#endif

#ifndef WITH_RX_XBEE
#define WITH_RX_XBEE 1
#endif

#ifndef WITH_WATCHDOG
#define WITH_WATCHDOG 0
#endif

#ifndef WITH_RF24_RX
#define WITH_RF24_RX 0
#endif

#ifndef WITH_LOGGER
#define WITH_LOGGER 0
#endif

#if WITH_TFT
//UTFT-Output
#include <UTFT.h>
#include <TFTStream.h>
#include <BayDebugTFT.h>

//UTFT myGLCD(ITDB18SP,33,32,31,35,34); //160x120 Serial
//36+37 for VCC + GND
#ifndef UTFT_AUTOOFF
UTFT myGLCD(ITDB24E_8, 38, 39, 40, 41); //240x320 Parallel
#define UTFT_AUTOOFF 120000 /*ms*/
#define utftcols 30
#define utftrows 26
#endif
char utftbuffer[utftrows * (utftcols + 1)];

BayTFTDebug TFT = BayTFTDebug(&myGLCD, utftbuffer, utftrows, utftcols);

#define UTFTprintP(x) utftprintPGM(PSTR(x))
#define UTFTprintlnP(x) utftprintlnPGM(PSTR(x))

uint8_t tft_output_rx=1;
#endif

#if WITH_RF24_RX
//RF24
#include <digitalWriteFast.h>
#include "iBoardRF24.h"
#ifndef RF24_PIPES
const uint64_t pipes[6] = {0x45c431ae12LL, 0x45c431ae24LL, 0x45c431ae48LL,
	0x45c431ae96LL, 0x45c431aeabLL, 0x45c431aebfLL
};
#endif

#ifndef RF24_CHANNEL
#define RF24_CHANNEL 0x71
#endif
//GBoard Pro
#ifndef RF24_RADIO
iBoardRF24 radio(12, 11, 8, 7, 9, 2);
#endif

void init_RF24(void) {
	radio.begin();
	radio.setChannel(RF24_CHANNEL);
	radio.setPayloadSize(32);
	radio.enableDynamicPayloads();
	radio.setCRCLength( RF24_CRC_16 );
	radio.setDataRate(RF24_250KBPS);
	radio.setPALevel(RF24_PA_MAX);
	radio.setRetries(15,15);
	radio.setAutoAck(true);
	radio.openWritingPipe(pipes[0]);
	for (uint8_t i = 0; i < 6; i++) {
		radio.openReadingPipe(i, pipes[i]);
	}
	radio.startListening();
}
#endif

#if WITH_WATCHDOG
//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
volatile uint8_t wdcount = 0;
ISR(WDT_vect) {
	wdcount++;
	if (wdcount > 30) {
		asm volatile (" jmp 0"); //restart programm
	}
}
#endif

/*************************************************
 Variables
 *************************************************/
#if WITH_RX_XBEE
uint16_t rx_panid;
/*
   Create a huge ring buffer to store incoming RX Packages while
   arduino is busy with GPRS...
*/
#define RX_BUFFER_SIZE 1024
unsigned char buffer[RX_BUFFER_SIZE];
#endif
unsigned long next_alive, next_send, next_try, pos, last_eeprom;
uint16_t rx_ok, rx_error, tx_error;
uint8_t rep_tx_error, tx_res;
uint8_t last_rx_rssi;

/*************************************************
 FUNCTIONS
 *************************************************/
#if WITH_TFT
volatile bool tft_switch = 0;
void tftOn(void) {
	tft_switch = 1;
}

unsigned long tft_autooff;

void utftprintPGM(const char *str) {
	char c;
	while (true) {
		c = pgm_read_byte(str);
		if (!c) break;
		TFT.write(c);
		str++;
	}
}

void utftprintlnPGM(const char *str) {
	utftprintPGM(str);
	TFT.println();
}

void tftSwitchOn(void) {
	//  pinMode(36, OUTPUT);
	//  pinMode(37, OUTPUT);
	//  digitalWrite(37,HIGH);
	//  digitalWrite(36,LOW);
	TFT.lcdOn();
	TFT.begin();
	TFT.flush();
	tft_autooff = millis() + UTFT_AUTOOFF;
}

void tftSwitchOff(void) {
	TFT.end();
	TFT.lcdOff();
	//digitalWrite(37,LOW);
}

#endif

void initRouter(void) {
#if WITH_TFT
	attachInterrupt(0, tftOn, CHANGE);
	digitalWrite(18, HIGH); //Pullup for Interrupt INT5
	attachInterrupt(5, tftOn, FALLING);
	tftSwitchOn();
	UTFTprintP("FW ");
	UTFTprintlnP(__DATE__);
	TFT.flush();
#endif
#if WITH_BAYEOS_LOGGER
	loggerclient.begin(LOGGER_BAUD_RATE);
#endif

#if WITH_RF24_RX
	init_RF24();
#endif

#if WITH_RX_XBEE
#if WITH_TFT
	UTFTprintlnP("Starting XBee... ");
	TFT.flush();
#endif
	//Replace the RX ring buffer with the larger one...
	RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE);
	xbee_rx.setSerial(RX_SERIAL);
	xbee_rx.begin(38400);
	while (!rx_panid) {
		rx_panid = xbee_rx.getPANID();
	}
#if WITH_TFT
	UTFTprintP("PANID: ");
	TFT.println(rx_panid);
	TFT.flush();
#endif
#endif
#if WITH_WATCHDOG
	Sleep.setupWatchdog(9); //init watchdog timer to 8 sec
#endif
}

#if WITH_RX_XBEE
void handle_RX_data(void) {
	if (RX_SERIAL.available()) {
#if WITH_TFT
		if(RX_SERIAL.available()>(RX_BUFFER_SIZE-100))
		tft_output_rx=0;
		else
		tft_output_rx=1;
		xbee_rx.readPacket();
#endif
		if (xbee_rx.getResponse().isAvailable()) {
			switch (xbee_rx.parseRX16(client, rx_panid)) {
			case 0:
				//ok
				rx_ok++;
				client.writeToBuffer();
#if WITH_TFT
				if (TFT.isOn() && tft_output_rx) {
					xbee_rx.parseRX16(TFT, rx_panid);
					TFT.sendPayload();
					TFT.flush();
				}
#endif

				break;
			case 1:
				rx_error++;
				break;
			case 2:
				break;
			};

		}
	}
}
#endif

#if WITH_RF24_RX

void handle_RF24(void) {
	uint8_t pipe_num, len;
	uint8_t payload[32];
	uint8_t count;
	while(radio.available(&pipe_num)) {
		if(len=radio.getDynamicPayloadSize()) {
			client.startRoutedFrame(pipe_num, 0);
			// Fetch the payload
			radio.read( payload, len );
			for (uint8_t i = 0; i < len; i++) {
				client.addToPayload(payload[i]);
			}
			client.writeToBuffer();
#if WITH_TFT
			if (TFT.isOn() && tft_output_rx) {
				TFT.startRoutedFrame(pipe_num, 0);
				for (uint8_t i = 0; i < len; i++) {
					TFT.addToPayload(payload[i]);
				}
				TFT.sendPayload();
				TFT.flush();
			}
#endif
		} else radio.read( payload, len);
		if(count>10) return;
	}
	delay(1);
}
#endif


void sendData(void){
    next_send = millis() + SENDING_INTERVAL;
    next_try = millis() + NEXT_TRY_INTERVAL;
    UTFTprintP("Sending ");
    if ( (tx_res = client.sendMultiFromBuffer()) ) {
#if WITH_TFT
      UTFTprintP("failed - ");
      TFT.println(tx_res);
#endif
      tx_error++;
      rep_tx_error++;

    } else {
      rep_tx_error = 0;
#if WITH_TFT
      UTFTprintlnP("OK");
#endif
    }
#if WITH_TFT
    TFT.flush();
#endif


}
