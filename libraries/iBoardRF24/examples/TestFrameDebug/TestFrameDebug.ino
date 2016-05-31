#include <iBoardRF24.h>
#include "printf.h"

//#define RF24ADDRESS 0x45c431ae12LL
#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
//#define RF24ADDRESS 0x45c431ae96LL
//#define RF24ADDRESS 0x45c431aeabLL
//#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x73


/* ce,csn pins - adjust to your layout*/
//iBoardRF24 radio(12, 11, 8, 7, 9, 2);
iBoardRF24 radio(A15, A11, A12, A13, A14, 2);


void setup(void){
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(0x71);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  delay(300);
  printf_begin(); 
  radio.printDetails();
}

void loop(void){
   
}

