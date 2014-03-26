#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>





BayRF24 client=BayRF24(8,9);


void setup(void){
  Serial.begin(9600);
  client.init(0xF0F0F0F0D2LL);
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   
   
    
  delay(5000);
   
}

