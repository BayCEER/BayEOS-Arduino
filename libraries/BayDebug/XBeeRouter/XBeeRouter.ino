/*
Router - XBee-Serial-Debug-Router 

Runs on Arduino Mega with more then one Serial

*/
#define RX_SERIAL Serial1

#include <HardwareSerialNew.h>
#include <XBee.h>
#include <BayXBee.h>
#include <BayEOS.h>
#include <BayDebug.h>

/*
 * Create a huge ring buffer to store incoming RX Packages while
 * arduino is busy 
 */
#define RX_BUFFER_SIZE 256
unsigned char buffer[RX_BUFFER_SIZE];


BayDebug client=BayDebug();

XBee xbee_rx = XBee();
Rx16Response rx16 = Rx16Response();


uint16_t rx_panid;
	
unsigned long last_alive_message;

void setup(void){
  RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE); 
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);

  while(! rx_panid){
    rx_panid=getPANID(xbee_rx);
  }
  
  
  client.begin(19200,1);  
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
          switch(parseRX16(client,xbee_rx,rx_panid)){
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

