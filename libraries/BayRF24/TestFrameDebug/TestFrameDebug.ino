#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>
#include "printf.h"



/* ce,csn pins - adjust to your layout*/
BayRF24 client=BayRF24(9,10); 


void setup(void){
  Serial.begin(9600);
  client.init(0x45c431ae12LL);
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

