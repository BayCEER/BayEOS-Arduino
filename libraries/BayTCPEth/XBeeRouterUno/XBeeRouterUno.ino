 /**
 Router - XBee-Ethernet with SD-Card Buffer
  Please note: 
  Depending on the size of the RX-Serial-Buffer there is a 
  it is possible that RX-Packets get lost, because RX-Serial-Buffer
  fills up while Arudino is sending.
  Normal buffer size is 64 byte.
  You can increase setting the SERIAL_BUFFER_SIZE in
  hardware/arduino/cores/arduino/HardwareSerial.cpp
  
  or use the modified HardwareSerial
  
 */
 
#include <HardwareSerialNew.h>

#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>
#include <Ethernet.h>
#include <SPI.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPEth.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
  
/* 
Please define your MAC, IP and Server, password
*/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
byte ip[] = { 132, 180, 112, 17 };
uint16_t rx_ok,rx_error,tx_error;
  
#define RX_SERIAL Serial
/*
 * Create a huge ring buffer to store incoming RX Packages while
 * arduino is busy with Ethernet
 */
#define RX_BUFFER_SIZE 200
unsigned char buffer[RX_BUFFER_SIZE];
  
BayEth client;
BayEOSBufferRAM  myBuffer;
  
XBee xbee_rx = XBee();
Rx16Response rx16 = Rx16Response();
uint16_t rx_panid;

unsigned long last_alive;

void setup(void){
  RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE); 
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);	
  client.readConfigFromStringPGM(
    PSTR("192.168.0.1|80|gateway/frame/saveFlat|admin|xbee|DEMO-Router|||||")
  );
  Ethernet.begin(mac, ip);
  while(! rx_panid) rx_panid=getPANID(xbee_rx);
  myBuffer=BayEOSBufferRAM(3000);
  client.setBuffer(myBuffer);

}
  
void loop(void){
  if(millis()-last_alive>30000){
    last_alive=millis();
      client.startDataFrame(BayEOS_Float32le);
      client.addToPayload((uint8_t) 0);
      client.addToPayload((float) millis()/1000);
      client.writeToBuffer();
      //client.writeToBuffer();
    }

  handle_RX_data();
  client.sendMultiFromBuffer();
}


void handle_RX_data(void){
    while(RX_SERIAL.available()){        
	xbee_rx.readPacket();

	if (xbee_rx.getResponse().isAvailable()) {
          switch(parseRX16(client,xbee_rx,rx_panid)){
            case 0:
             //ok
             rx_ok++;
             client.sendOrBuffer();
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

