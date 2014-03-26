#include <SPI.h>
#include "RF24.h"

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(8,9);

// Topology
const uint64_t talking_pipes[5] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0B4LL, 0xF0F0F0F0A5LL, 0xF0F0F0F096LL };
const uint64_t listening_pipes[5] = { 0x3A3A3A3AD2LL, 0x3A3A3A3AC3LL, 0x3A3A3A3AB4LL, 0x3A3A3A3AA5LL, 0x3A3A3A3A96LL };

uint8_t payload[32];
uint8_t len=0;
void setup(void){
   radio.begin();
   radio.enableDynamicPayloads();
   radio.setCRCLength( RF24_CRC_16 ) ;
   radio.setDataRate(RF24_250KBPS);
   radio.setPALevel(RF24_PA_HIGH);
   radio.openWritingPipe(talking_pipes[0]);
   radio.powerDown() ;
}

void loop(void){
   radio.powerUp() ;
   radio.stopListening();
   payload[0]=0x3; //int16
   payload[1]=0x1;
   payload[2]=0;
   *(int*)(payload+3)=10;
   *(int*)(payload+5)=analogRead(A1);
   len=7;
   radio.write( payload, len);
  // delay(2);
   radio.powerDown() ;
   delay(2000);
}
