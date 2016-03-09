#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>
#include "printf.h"

//#define RF24ADDRESS 0x45c431ae12LL
//#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
//#define RF24ADDRESS 0x45c431ae96LL
//#define RF24ADDRESS 0x45c431aeabLL
#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x61


/* ce,csn pins - adjust to your layout*/
BayRF24 client=BayRF24(9,10); 


void setup(void){
  Serial.begin(9600);
  delay(300);
  client.init(RF24ADDRESS,RF24CHANNEL);
  printf_begin(); 
  client.printDetails();
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   if(client.sendPayload())
     Serial.println("failed");
   else
     Serial.println("ok");
  delay(5000);
   
}

