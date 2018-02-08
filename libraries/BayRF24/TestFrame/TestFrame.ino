#include <BayRF24.h>

#define RF24ADDRESS 0x45c431ae12LL
//#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
//#define RF24ADDRESS 0x45c431ae96LL
//#define RF24ADDRESS 0x45c431aeabLL
//#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x71




BayRF24 client=BayRF24(9,10);


void setup(void){
  Serial.begin(9600);
  client.init(RF24ADDRESS,RF24CHANNEL);
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

