#include <SPI.h>
#include "RF24.h"
#include "printf.h"

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(8,9);

// Topology

uint8_t payload[32];
uint8_t len=0;
void setup(void){
   Serial.begin(9600);
   printf_begin();
   
   radio.begin();
   radio.enableDynamicPayloads();
   radio.setCRCLength( RF24_CRC_16 ) ;
   radio.setDataRate(RF24_250KBPS);
   radio.setPALevel(RF24_PA_HIGH);
   radio.openWritingPipe(0x45c431ae12LL);
   radio.printDetails();
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
