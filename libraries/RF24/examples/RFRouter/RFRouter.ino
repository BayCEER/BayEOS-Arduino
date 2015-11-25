/*

BayEOSnRF24-Router

*/

#include <BayEOS.h>
#include <BayDebug.h>
#include <SPI.h>
#include <RF24.h>
BayDebug client=BayDebug();

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the nodes to communicate.  Only ping nodes need
// dedicated pipes in this topology.  Each ping node has a talking pipe
// that it will ping into, and a listening pipe that it will listen for
// the pong.  The pong node listens on all the ping node talking pipes
// and sends the pong back on the sending node's specific listening pipe.

const uint64_t pipes[6] = { 0x45c431ae12LL,0x45c431ae24LL, 0x45c431ae48LL, 
    0x45c431ae9fLL, 0x45c431aeabLL, 0x45c431aebfLL };


void setup(void){
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
   
   client.begin(9600,1);  
   radio.printDetails();
  client.sendMessage("RF24-Router started");
 
}

uint8_t payload[32];
void loop(void){
    uint8_t pipe_num, len;
    if ( radio.readPipe(payload,&pipe_num) ){
      client.startRoutedFrame(pipe_num,0);
      for(uint8_t i=0; i<len;i++){
	  client.addToPayload(payload[i]);
      }
      client.sendPayload();
      

    }
 
}
