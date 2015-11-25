/*
IBoardPro XBeeRouter


SPEC:
XBee: Serial3
SD:CS: 4

 Router - XBee-Ethernet with SD-Card Buffer and Watchdog 


*/


#define WITH_RF24_RX 0

#include <HardwareSerialNew.h>

#include <EEPROM.h>
#include <XBee.h>
#include <BayXBee.h>
#include <BayEOS.h>
#include <SPI.h>
#include <Base64.h>
#include <BayTCP.h>
#include <Ethernet.h>
#include <BayTCPEth.h>
#include <BayDebug.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <Sleep.h>
#include <SoftwareSerial.h>
#include <Arduino.h>


//UTFT-Output
#include <UTFT.h>
#include <TFTStream.h>
#include <BayDebugTFT.h>


#if WITH_RF24_RX
//RF24
#include <digitalWriteFast.h>
#include "iBoardRF24.h"
//IBoard Pro
iBoardRF24 radio(12,11,8,7,9,2);
const uint64_t pipes[6] = { 0x45c431ae12LL,0x45c431ae24LL, 0x45c431ae48LL, 
    0x45c431ae9fLL, 0x45c431aeabLL, 0x45c431aebfLL };
#endif


//UTFT myGLCD(ITDB18SP,35,34,33,37,36);   //120x160 - IBoard pro Serial Interface 
UTFT myGLCD(ITDB24E_8,38,39,40,41); //240x320 Parallel
#define UTFT_AUTOOFF 120000 /*ms*/
#define utftcols 30
#define utftrows 26
char utftbuffer[utftrows*(utftcols+1)];

BayTFTDebug TFT=BayTFTDebug(&myGLCD,utftbuffer,utftrows,utftcols);

#define UTFTprintP(x) utftprintPGM(PSTR(x))
#define UTFTprintlnP(x) utftprintlnPGM(PSTR(x))

#define SENDING_INTERVAL 30000
#define KEEPALIVE_INTERVAL 120000
#define RX_SERIAL Serial3

BayEth client;
byte mac[6];
byte ip[4];
byte mask[4];
byte default_gw[4];

//BayDebug client;
XBee xbee_rx = XBee();
BayEOSBufferSDFat myBuffer;

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
volatile bool tft_switch=0;
void tftOn(void){
  tft_switch=1;
}

unsigned long tft_autooff;

void utftprintPGM(const char *str){
	char c;
	while (true) {
		c=pgm_read_byte(str);
		if (!c) break;
	    TFT.write(c);
	    str++;
	}
}

void utftprintlnPGM(const char *str){
	utftprintPGM(str);
	TFT.println();
}

void tftSwitchOn(void){
//  pinMode(36, OUTPUT);
//  pinMode(37, OUTPUT);
//  digitalWrite(37,HIGH);
//  digitalWrite(36,LOW);
  TFT.lcdOn();
  TFT.begin();
  TFT.flush();
  tft_autooff=millis()+UTFT_AUTOOFF;
}

void tftSwitchOff(void){
  TFT.end();
  TFT.lcdOff();
  //digitalWrite(37,LOW);
}


void handle_RX_data(void){
  uint8_t count;
  while(RX_SERIAL.available()){        
    xbee_rx.readPacket();

    if (xbee_rx.getResponse().isAvailable()) {
        switch(parseRX16(client,xbee_rx,rx_panid)){
          case 0:
           //ok
           rx_ok++;
           client.writeToBuffer();
           if(TFT.isOn()){
             parseRX16(TFT,xbee_rx,rx_panid);
             TFT.sendPayload();
             TFT.flush(); 
           }

          break;
         case 1:
           rx_error++;
          break;
         case 2: 
          break; 
        };
    }
    count++;
    if(count>20){
      client.startFrame(BayEOS_ErrorMessage);
      client.addToPayload("Warning - Too much RX-Data");
      client.writeToBuffer();
      return;
    }
  }
  
}


#if WITH_RF24_RX 
void handle_RF24(void){
    uint8_t pipe_num, len;
    uint8_t payload[32]; 
    if ( len=radio.readPipe(payload,&pipe_num) ){
      //Note: RF24 is handelt like XBee with PANID0
      client.startRoutedFrame(pipe_num,0);
      for(uint8_t i=0; i<len;i++){
	  client.addToPayload(payload[i]);
      }
      client.writeToBuffer();
      if(TFT.isOn()){
        TFT.startRoutedFrame(pipe_num,0);
        for(uint8_t i=0; i<len;i++){
	  TFT.addToPayload(payload[i]);
        }

        TFT.sendPayload();
        TFT.flush(); 
      }
      
   }
 
}
#endif

void setup(void) {
//  Serial.begin(38400);
  attachInterrupt(0,tftOn,CHANGE);
  digitalWrite(18,HIGH); //Pullup for Interrupt INT5
  attachInterrupt(5,tftOn,FALLING);

  tftSwitchOn();
  UTFTprintP("FW ");
  UTFTprintlnP(__DATE__);
  TFT.flush();
 
  UTFTprintlnP("Starting XBee... ");
  TFT.flush();
 //Replace the RX ring buffer with the larger one...
  RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE);
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);
  while (!rx_panid) {
	rx_panid = getPANID(xbee_rx);
  }
  UTFTprintP("PANID: ");
  TFT.println(rx_panid);
  TFT.flush();
   
  startupframe=1;
  startupsend=1;
  Sleep.setupWatchdog(9); //init watchdog timer to 8 sec

  if (!SD.begin(4)) {
    UTFTprintlnP("No SD.");
    TFT.flush();
    delay(1000);
    return;
  }

  UTFTprintlnP("READ config");
  TFT.flush();

  client.readConfigFromFile("ETH.TXT");
  for(uint8_t i=1;i<=10;i++){
    TFT.print(i);
    UTFTprintP(": ");
    TFT.println(*client.getConfigPointer((i-1)));
  }
  TFT.flush();

  memcpy(mac,client.parseMAC(*client.getConfigPointer(BayTCP_CONFIG_MAC)),6);
  memcpy(ip,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_IP)),4);
  memcpy(mask,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_MASK)),4);
  memcpy(default_gw,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_DEFAULT_GW)),4);
    
  if(ip[0])
    Ethernet.begin(mac, ip, default_gw, default_gw, mask);
  else
    Ethernet.begin(mac);
  
  if(*(*client.getConfigPointer(1)+2)=='9'){
     client._urlencode=0;
     UTFTprintlnP("Using old port 8090 without urlencode");
     TFT.flush();
  }


  myBuffer = BayEOSBufferSDFat(2000000000, 1); //Append mode!
  UTFTprintlnP("Buffer Ok");
  UTFTprintP("Size: ");
  pos=myBuffer.readPos();
  TFT.println(pos);
  TFT.flush();
 
  client.setBuffer(myBuffer, 0);

  UTFTprintlnP("Setup ok :-)");
  TFT.flush();
  
  client.startFrame(BayEOS_Message);
  client.addToPayload("Router started - FW ");
  client.addToPayload(__DATE__);
  client.writeToBuffer();

  tft_autooff=millis()+UTFT_AUTOOFF;
  

#if WITH_RF24_RX == 1
   radio.begin();
   radio.setChannel(0x71);
   radio.enableDynamicPayloads();
//   radio.setCRCLength( RF24_CRC_16 ) ;
   radio.setDataRate(RF24_250KBPS);
   radio.setPALevel(RF24_PA_HIGH);
   for(uint8_t i=0;i<6;i++){
     radio.openReadingPipe(i,pipes[i]);
   }
   radio.startListening();
#endif

}

void loop(void) {
	wdcount = 0; //clear watchdog count!

        if((tft_autooff-millis())>UTFT_AUTOOFF){
          tftSwitchOff();
        }
        
        if(tft_switch){
          tft_switch=0;
          if(! TFT.isOn())  tftSwitchOn();
          tft_autooff=millis()+UTFT_AUTOOFF;
            
        }
        
             
	if ((millis() - last_alive) > KEEPALIVE_INTERVAL || startupframe) {
                startupframe=0;
		last_alive = millis();
		client.startDataFrame(BayEOS_Float32le);
		client.addChannelValue(millis() / 1000);
		client.addChannelValue(myBuffer.writePos());
		client.addChannelValue(myBuffer.readPos());
		client.addChannelValue(tx_error);
		client.addChannelValue(rx_ok);
		client.addChannelValue(rx_error);
  	        client.writeToBuffer();
        }
        
	if (  ((millis() - last_send) > SENDING_INTERVAL || myBuffer.available()>2000 || startupsend)) {
		last_send = millis();
        
		UTFTprintP("Sending ");
		if ( (tx_res=client.sendMultiFromBuffer()) ){
			UTFTprintP("failed - ");
                        TFT.println(tx_res);
                        tx_error++;
                        rep_tx_error++;
                        
                } else {
                        rep_tx_error=0;
                        startupsend=0;
			UTFTprintlnP("OK");

                }
                TFT.flush();
 
	}
        
   handle_RX_data();
 
#if WITH_RF24_RX == 1 
   handle_RF24();
#endif
  

}




