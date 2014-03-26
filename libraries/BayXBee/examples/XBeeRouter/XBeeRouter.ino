/*----------------------------------------------------
Simple BayXBee router designed for Arduino-Mega
using two serial ports and a RAM for buffering
when destination XBee is not available.
----------------------------------------------------*/

#define RX_SERIAL Serial
#define TX_SERIAL Serial1 
/*
To save ram you can define RX_PANID here.
The programm will set rx_panid without querying the bee

#define RX_PANID 10002
*/


#include <HardwareSerialNew.h>

#include <XBee.h>
#include <BayEOS.h>
#include <BayXBee.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>

/*
 * Create a huge ring buffer to store incoming RX Packages while
 * arduino is busy 
 */
#define RX_BUFFER_SIZE 256
unsigned char buffer[RX_BUFFER_SIZE];


XBee xbee_rx = XBee();
BayXBee xbee_tx = BayXBee();

BayEOSBufferRAM  myBuffer;

// create reusable response objects for responses we expect to handle 
Rx16Response rx16 = Rx16Response();


uint8_t data = 0;
uint8_t i;
uint16_t rx_panid;

void setup() {
  RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE); 
  xbee_tx.setSerial(TX_SERIAL); 
  xbee_rx.setSerial(RX_SERIAL);
  xbee_tx.begin(38400);
  xbee_rx.begin(38400);
  
  myBuffer=BayEOSBufferRAM(3000);
  xbee_tx.setBuffer(myBuffer);
  
#if RX_PANID
  rx_panid=RX_PANID;
#else
  while(! rx_panid) rx_panid=getPANID(xbee_rx);
#endif
}



void loop() {
    handle_RX_data();
    if(xbee_tx.sendFromBuffer()) delay(500); 
 }


 void handle_RX_data(void){
    while(RX_SERIAL.available()){        
	xbee_rx.readPacket();

	if (xbee_rx.getResponse().isAvailable()) {
          switch(parseRX16(xbee_tx,xbee_rx,rx_panid)){
            case 0:
             //ok
             xbee_tx.sendOrBuffer();
            break;
           case 1:
             xbee_tx.sendError("Parse Error 1");
            break;
           case 2: 
             xbee_tx.sendError("Parse Error 2");
            break; 
          };
      }
    }
  
}

