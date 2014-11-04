/*
GBoardPro XBeeRouter



SPEC:
XBee: Serial2
GPRS: Serial1 RST: 47 PWR: 46
Logger: Serial 
SD:CS: 4

 Router - XBee-GPRS with SD-Card Buffer and Watchdog 


 
 */

#include <HardwareSerialNew.h>
 
#include <EEPROM.h>
#include <XBee.h>
#include <BayXBee.h>
#include <BayEOS.h>
#include <SPI.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>
#include <BayDebug.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <Sleep.h>
#include <SoftwareSerial.h>
#include <Arduino.h>



#define DEBUG_AUTOOFF 120000 /*ms*/

BayDebug DEBUG=BayDebug();
boolean DEBUGisOn;

#define DEBUGprintP(x) DEBUGprintPGM(PSTR(x))
#define DEBUGprintlnP(x) DEBUGprintlnPGM(PSTR(x))

#define SENDING_INTERVAL 60000
#define RX_SERIAL Serial2
#define TX_SERIAL Serial1

BayGPRS client = BayGPRS(TX_SERIAL,9);


//BayDebug client;
XBee xbee_rx = XBee();
BayEOSBufferSDFat myBuffer;
RTC_SIM900 myRTC;

uint16_t rx_panid;

/*
 * Create a huge ring buffer to store incoming RX Packages while
 * arduino is busy with GPRS...
 */
#define RX_BUFFER_SIZE 1024
unsigned char buffer[RX_BUFFER_SIZE];

unsigned long last_alive, last_send, pos, last_eeprom;
uint16_t rx_ok,rx_error,tx_error;
uint8_t rep_tx_error, tx_res;
uint8_t last_rx_rssi;
uint8_t startupframe, startupsend;

//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
volatile uint8_t wdcount = 0;
ISR(WDT_vect) {
	wdcount++;
	if(wdcount>30){
          asm volatile (" jmp 0"); //restart programm
        }
}

/*************************************************
   FUNCTIONS
*************************************************/
volatile bool DEBUG_switch=0;
void DEBUGOn(void){
  DEBUG_switch=1;
}

unsigned long DEBUG_autooff;

void DEBUGprintPGM(const char *str){
	char c;
	while (true) {
		c=pgm_read_byte(str);
		if (!c) break;
	    Serial.write(c);
	    str++;
	}
}

void DEBUGprintlnPGM(const char *str){
	DEBUGprintPGM(str);
	Serial.println();
}

void DEBUGSwitchOn(void){
  DEBUG.begin(9600);
  DEBUG_autooff=millis()+DEBUG_AUTOOFF;
  DEBUGisOn=1;
}

void DEBUGSwitchOff(void){
  //digitalWrite(37,LOW);
  DEBUGisOn=0;
}

void handle_RX_data(void){
    while(RX_SERIAL.available()){        
	xbee_rx.readPacket();

	if (xbee_rx.getResponse().isAvailable()) {
          switch(parseRX16(client,xbee_rx,rx_panid)){
            case 0:
             //ok
             rx_ok++;
             client.writeToBuffer();
             if(DEBUGisOn){
               parseRX16(DEBUG,xbee_rx,rx_panid);
               DEBUG.sendPayload();
             }

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


void setup(void) {
  attachInterrupt(0,DEBUGOn,CHANGE);
  digitalWrite(18,HIGH); //Pullup for Interrupt INT5
  attachInterrupt(5,DEBUGOn,FALLING);

  DEBUGSwitchOn();
  DEBUGprintP("FW ");
  DEBUGprintlnP(__DATE__);

#if WITH_BAYEOS_LOGGER
  loggerclient.begin(LOGGER_BAUD_RATE);
#endif
 
  DEBUGprintlnP("Starting XBee... ");
 //Replace the RX ring buffer with the larger one...
  RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE);
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);
  while (!rx_panid) {
	rx_panid = getPANID(xbee_rx);
  }
  DEBUGprintP("PANID: ");
  Serial.println(rx_panid);
  
  startupframe=1;
  startupsend=1;
  Sleep.setupWatchdog(9); //init watchdog timer to 8 sec


  if (!SD.begin(4)) {
    DEBUGprintlnP("No SD.");
    delay(1000);
    return;
  }

  DEBUGprintlnP("READ config");

  client.readConfigFromFile("GPRS.TXT");
  for(uint8_t i=1;i<=10;i++){
    Serial.print(i);
    DEBUGprintP(": ");
    Serial.println(*client.getConfigPointer((i-1)));
  }

//  client.softSwitch();
  if(*(*client.getConfigPointer(1)+2)=='9'){
     client._urlencode=0;
     DEBUGprintlnP("Using old port 8090 without urlencode");
  }


#if WITH_BAYEOS_LOGGER
  //Write Name to EEPROM (for BayLogger!)
  EEPROM.write(EEPROM_NAME_OFFSET, EEPROM_NAME_STARTBYTE);
  uint8_t i=0;
  char c;
  while(c=*(*client.getConfigPointer(2)+i)){
     EEPROM.write(EEPROM_NAME_OFFSET + i + 1, c);
     i++;
  }
  EEPROM.write(EEPROM_NAME_OFFSET + i + 1, 0);
#endif

  DEBUGprintP("Starting GPRS...");
  switch(client.begin(38400)){
    case 0:
      DEBUGprintlnP("OK");
      break;
    case 1:
      DEBUGprintlnP("Communication Error");
      break;
    case 2:
      DEBUGprintlnP("PIN Error");
      break;
    case 3:
      DEBUGprintlnP("PIN Locked");
      break;
    case 4:
      DEBUGprintlnP("No Network");
      break;
    case 5:
      DEBUGprintlnP("No GPRS");
      break;
    case 6:
      DEBUGprintlnP("No SIM Card");
      break;
  }
 
  myRTC.adjust(client.now());

  myBuffer = BayEOSBufferSDFat(2000000000, 1); //Append mode!
  DEBUGprintlnP("Buffer Ok");
  DEBUGprintP("Size: ");
  pos=myBuffer.readPos();
  Serial.println(pos);
  
 
  myBuffer.setRTC(myRTC, 0); //Relative Mode...
  client.setBuffer(myBuffer, 0);
#if WITH_BAYEOS_LOGGER
  loggerclient.setBuffer(myBuffer);
  myLogger.init(loggerclient,myBuffer,myRTC);
#endif
  myBuffer.seekReadPointer(pos); //Logger.init moves Read pointer!
  
  
  DEBUGprintP("RSSI:");
  Serial.println(client.getRSSI());

  DEBUGprintlnP("Setup ok :-)");
  
  client.startFrame(BayEOS_Message);
  client.addToPayload("Router started - FW");
  client.addToPayload(__DATE__);
  client.writeToBuffer();

  DEBUG_autooff=millis()+DEBUG_AUTOOFF;
  

#if WITH_RF24_RX == 1
   radio.begin();
   radio.setChannel(0x71);
   radio.enableDynamicPayloads();
//   radio.setCRCLength( RF24_CRC_16 ) ;
   radio.setDataRate(RF24_250KBPS);
   radio.setPALevel(RF24_PA_HIGH);
   radio.openReadingPipe(1,pipes[0]);
   radio.openReadingPipe(2,pipes[1]);
   radio.openReadingPipe(3,pipes[2]);
   radio.openReadingPipe(4,pipes[3]);
   radio.openReadingPipe(5,pipes[4]);
   radio.startListening();
#endif

}

void loop(void) {
	wdcount = 0; //clear watchdog count!

        if((DEBUG_autooff-millis())>DEBUG_AUTOOFF){
          DEBUGSwitchOff();
        }
        
        if(DEBUG_switch){
          DEBUG_switch=0;
          if(! DEBUGisOn)  DEBUGSwitchOn();
          DEBUG_autooff=millis()+DEBUG_AUTOOFF;
            
        }
        
        #if WITH_BAYEOS_LOGGER
        //Save read pos every hour
        if( ((millis() - last_eeprom)> 3600000 )  || startupframe){
           pos = myBuffer.readPos(); 
 	   for (uint8_t i = 0; i < 4; i++) {
		EEPROM.write(EEPROM_READ_POS_OFFSET + i, *(&pos + i));
           }
           last_eeprom= millis();        
        } 
        #endif
            
	if ((millis() - last_alive) > SENDING_INTERVAL || startupframe) {
                startupframe=0;
		last_alive = millis();
		client.startDataFrame(BayEOS_Float32le);
		client.addChannelValue(millis() / 1000);
		client.addChannelValue(myBuffer.writePos());
		client.addChannelValue(myBuffer.readPos());
		client.addChannelValue(tx_error);
		client.addChannelValue(rx_ok);
		client.addChannelValue(rx_error);
                client.addChannelValue(-1);
                client.addChannelValue(myRTC.now().get());
                client.addChannelValue(client.now().get());
                client.addChannelValue(client.getRSSI());
                client.addChannelValue((float)analogRead(A15)/1023*5*10);
  	        client.writeToBuffer();
        }
        
	if (
  ((millis() - last_send) > SENDING_INTERVAL || myBuffer.available()>2000 || startupsend)) {
		last_send = millis();
        
		DEBUGprintP("Sending ");
		if ( (tx_res=client.sendMultiFromBuffer()) ){
			DEBUGprintP("failed - ");
                        Serial.println(tx_res);
                        tx_error++;
                        rep_tx_error++;
                        
                } else {
                        rep_tx_error=0;
                        startupsend=0;
			DEBUGprintlnP("OK");
                        unsigned long simTime=client.now().get();
                        unsigned long rtcTime=myRTC.now().get();
                        if((simTime>rtcTime && (simTime-rtcTime)<120)
                         || (simTime>rtcTime && (rtcTime-simTime)<120) ){
                           DEBUGprintlnP("setting RTC");
		           myRTC.adjust(DateTime(simTime));
                         } 

                }
 
                if(rep_tx_error%5==4){
                  client.softSwitch();
                  client.startFrame(BayEOS_Message);
                  client.addToPayload("TX-ERROR SoftSwitch");
                  client.writeToBuffer();

                }
	}
        
   handle_RX_data();
 

}




