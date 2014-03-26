 /**
 Router - XBee-Ethernet with SD-Card Buffer
  Please note: 
  Depending on the size of the RX-Serial-Buffer there is a 
  it is possible that RX-Packets get lost, because RX-Serial-Buffer
  fills up while Arudino is sending.
  Normal buffer size is 64 byte.
  
  Note this sketch does not run on atmega328. Insufficient RAM!!
  
 */
#include <HardwareSerialNew.h>
#include <BayEOS.h>
#include <SdFat.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferSDFat.h>
#include <XBee.h>
#include <BayXBee.h>
#include <Ethernet.h>
#include <SPI.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPEth.h>
  
byte mac[6];
byte ip[4];
byte mask[4];
byte default_gw[4];
uint16_t rx_ok,rx_error,tx_error;
  
#define RX_SERIAL Serial3
/*
 * Create a huge ring buffer to store incoming RX Packages while
 * arduino is busy with Ethernet
 */
#define RX_BUFFER_SIZE 1024
unsigned char buffer[RX_BUFFER_SIZE];
  
BayEth client;
  
XBee xbee_rx = XBee();
BayEOSBufferSDFat myBuffer;
Rx16Response rx16 = Rx16Response();
uint16_t rx_panid;

unsigned long last_alive;

void setup(void){
  
  RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE); 
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);	


//Use 4 for Ethernet-Shield
// "A11" for Mega-XBee-Shield
  if (!SD.begin(4)) {
  	//This should not happen!!
  }

  client.readConfigFromFile("ETH.TXT");
  
  memcpy(mac,client.parseMAC(*client.getConfigPointer(BayTCP_CONFIG_MAC)),6);
  memcpy(ip,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_IP)),4);
  memcpy(mask,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_MASK)),4);
  memcpy(default_gw,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_DEFAULT_GW)),4);
    
  Ethernet.begin(mac, ip, default_gw, default_gw, mask);

  myBuffer = BayEOSBufferSDFat(100000000); 
    
  while(! rx_panid) rx_panid=getPANID(xbee_rx);

  client.setBuffer(myBuffer);
}
  
void loop(void){
  if(millis()-last_alive>30000 || myBuffer.available()>5000){
    last_alive=millis();
      client.startDataFrame(BayEOS_Float32le);
      client.addToPayload((uint8_t) 0);
      client.addToPayload((float) millis()/1000);
      client.sendPayload();
      //client.writeToBuffer();
      client.sendMultiFromBuffer();
    }

  handle_RX_data();
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

