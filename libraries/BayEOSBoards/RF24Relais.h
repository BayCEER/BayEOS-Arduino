#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
#include <BayEOS.h>
#include <BayEOSCommands.h>
#include <RF24.h>
#include <BayRF24.h>

#ifdef __AVR_ATmega168P__
uint8_t buffer[100];
#else
uint8_t buffer[1000];
#endif
BayEOSBufferRAM myBuffer(buffer);
BayRF24 client = BayRF24(9, 10, 0); //Without Power down

#ifndef WITH_SERIAL
#define WITH_SERIAL 0
#endif

#if WITH_SERIAL
#include <BaySerial.h>
bool serial_update=false;
BaySerial serial_client(Serial,100,9600);
#endif

#define R1_PIN 8

uint8_t r1 = 0;
uint8_t command, channel;
uint16_t arg;
uint8_t pipe_num, len;
uint8_t nrf_payload[32];
uint16_t rx_count, rx_error;
unsigned long last_buffer, last_data;
unsigned long last_command, ontime;

void sendCommand(uint8_t command, uint8_t channel = 0x01, uint8_t arg = 0) {
	client.startFrame(BayEOS_ChecksumFrame);
	client.addToPayload((uint8_t) BayEOS_Command);
	client.addToPayload((uint8_t) BayEOS_SwitchCommand);
	client.addToPayload(command);
	client.addToPayload(channel);
	client.addToPayload(arg);
	client.addChecksum();
	client.sendPayload();
}

#if WITHINT0SWITCH
uint8_t int0;
void int0SwitchHandler(void);
#endif

#if WITHINT1SWITCH
uint8_t int1;
void int1SwitchHandler(void);
#endif

void commandHandler(void);

void initBoard() {
	pinMode(R1_PIN, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	client.init(RF24ADDRESS, RF24CHANNEL);
	client.setRetries(15, 15);
	for (uint8_t i = 0; i < 6; i++) {
		client.openReadingPipe(i, pipes[i]);
	}
	client.startListening();
	client.setBuffer(myBuffer);
	digitalWrite(LED_BUILTIN, HIGH);

#if WITHINT0SWITCH
	digitalWrite(2, HIGH);
	int0 = digitalRead(2);
#endif

#if WITHINT1SWITCH
	digitalWrite(3, HIGH);
	int1 = digitalRead(3);
#endif

#if WITH_SERIAL
	serial_client.setBuffer(myBuffer);
	serial_client.begin(9600);
#endif
	ontime = MAXONTIME;
}

void runBoard(void) {

	if (r1 && (millis() - last_command) > ontime) {
		command = SWITCH_OFF;
		channel = 0x3;
		pipe_num = 0;
	}


	//Send from Buffer
	if ((millis() - last_buffer) > 2000) {
		last_buffer = millis();
#if WITH_SERIAL
		serial_client.sendFromBuffer();
#else
		client.sendFromBuffer();
#endif
	}

#if WITH_SERIAL
	if ((millis() - last_data) > 15000 || serial_update) {
		last_data = millis();
		serial_update=false;
	    serial_client.startDataFrame(BayEOS_LabelledChannelFloat32le);
	    serial_client.addChannelValue(r1,"Relais");
	    serial_client.addChannelValue((r1?(float)(ontime-(millis()-last_command))/60000:0), "Time");
	    serial_client.addChannelValue(rx_count,"RX");
	    serial_client.addChannelValue(rx_error,"RX-Error");
		serial_client.sendOrBuffer();
		rx_count = 0;
		rx_error = 0;
	}
#else

	if ((millis() - last_data) > 600000) {
		last_data = millis();
		client.startDataFrameWithOrigin(BayEOS_Int16le, STATUS_NAME, 1);
		client.addChannelValue(r1 * 100);
		client.addChannelValue(rx_count);
		client.addChannelValue(rx_error);
		client.addChecksum();
		client.sendOrBuffer();
		rx_count = 0;
		rx_error = 0;
	}
#endif

	uint8_t scount;
#if WITHINT0SWITCH
	scount = 0;
	while (scount < 20 && int0 != digitalRead(2)) {
		scount++;
		delay(10);
	}

	if (scount == 20 && int0 != digitalRead(2)) {
		int0 = digitalRead(2);
		int0SwitchHandler();
	}
#endif

#if WITHINT1SWITCH
	scount = 0;
	while (scount < 20 && int1 != digitalRead(3)) {
		scount++;
		delay(10);
	}

	if (scount == 20 && int1 != digitalRead(3)) {
		int1 = digitalRead(3);
		int1SwitchHandler();
	}
#endif

#if WITH_SERIAL
	if(Serial.available()){
	    if (! serial_client.readIntoPayload()) {
	    	if (serial_client.getPayload(0) == BayEOS_Command
	    						&& serial_client.getPayload(1) == BayEOS_SwitchCommand) {
	    					command = serial_client.getPayload(2);
	    					channel = serial_client.getPayload(3);
	    					arg=serial_client.getPayload(4);
	    				    if(serial_client.getPayloadLength()>4) arg+=256*serial_client.getPayload(5);
	    				    serial_update=true;
	    	}
	    }
	}
#endif


	while (client.i_available(&pipe_num)) {
		len = client.getDynamicPayloadSize();
		if (!len) {
			delay(5);
			continue;
		}
		client.read(nrf_payload, len);
		client.startFrame(nrf_payload[0]);
		for (uint8_t i = 1; i < len; i++) {
			client.addToPayload(nrf_payload[i]);
		}
		if (client.validateChecksum() == 0) {
			rx_count++;

			if (nrf_payload[1] == BayEOS_Command
					&& nrf_payload[2] == BayEOS_SwitchCommand) {
				command = nrf_payload[3];
				channel = nrf_payload[4];
				arg = nrf_payload[5];
#ifdef RF24FORWARD
				if (pipe_num == 2) {
					client.setTXAddr(RF24FORWARD);
					client.sendPayload();
					client.setTXAddr(RF24ADDRESS);
					command = 0;
				}
#endif
			} else if (nrf_payload[1] == BayEOS_OriginFrame) {
#if WITH_SERIAL
				serial_client.startFrame(client.getPayload(0));
				serial_client.addToPayload(client.getPayload()+1,client.getPacketLength()-1);
				serial_client.sendOrBuffer();
#else
				client.sendOrBuffer();
#endif
			}
		} else
			rx_error++;
	}
	commandHandler();
	delay(5);
}
