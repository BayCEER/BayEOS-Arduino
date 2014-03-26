#include <SPI.h>
#include <digitalWriteFast.h>
#include "iBoardRF24.h"
#include <BayEOS.h>
#include <BayDebug.h>

iBoardRF24 radio(12,11,8,7,9,2);//GBoard Pro
BayDebug client=BayDebug();

const uint64_t pipes[5] = { 0x45c431ae12LL,0x45c431ae24LL, 0x45c431ae48LL, 0x45c431ae9fLL, 0x45c431aeabLL };

void setup(void){
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
   
   client.begin(57600,1);  
  client.sendMessage("XBee-Router started");
   
 
}

uint8_t payload[32];
void loop(void){
    uint8_t pipe_num, len;
    if ( radio.available(&pipe_num) ){
      bool done = false;
      len = radio.getDynamicPayloadSize();
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
	len = radio.getDynamicPayloadSize();
	done = radio.read( payload, len );

        

      }
      client.startRoutedFrame(pipe_num,0);
      for(uint8_t i=0; i<len;i++){
	  client.addToPayload(payload[i]);
      }
      client.sendPayload();
      

    }
 
}
