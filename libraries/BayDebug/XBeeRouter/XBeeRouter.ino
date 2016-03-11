/*
Router - XBee-Serial-Debug-Router 

Runs on Arduino Mega with more then one Serial

*/
#define RX_SERIAL Serial1

#include <XBee.h>
#include <BayXBee.h>
#include <BayEOS.h>
#include <BayDebug.h>


BayDebug client(Serial);

XBee xbee_rx = XBee();
Rx16Response rx16 = Rx16Response();


uint16_t rx_panid;
	
unsigned long last_alive_message;

void setup(void){
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);

  while(! rx_panid){
    rx_panid=xbee_rx.getPANID();
  }
  
  
  client.begin(9600,1);  
  client.sendMessage("XBee-Router started");

}

void loop(void){
   if((millis()-last_alive_message)>10000){
	last_alive_message=millis();
	client.startDataFrame(BayEOS_Float32le);
	client.addToPayload((uint8_t) 0);
	client.addToPayload((float)(millis()/1000));
	client.sendPayload();	
    }
    
    handle_RX_data();
 }
 
 void handle_RX_data(void){
    while(RX_SERIAL.available()){        
	xbee_rx.readPacket();

	if (xbee_rx.getResponse().isAvailable()) {
          switch(xbee_rx.parseRX16(client,rx_panid)){
            case 0:
             //ok
             client.sendPayload();
            break;
           case 1:
             client.sendError("Parse Error 1");
            break;
           case 2: 
             client.sendError("Parse Error 2");
            break; 
          };
      }
    }
  
}

