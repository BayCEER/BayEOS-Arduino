#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>

#define RF24ADDRESS 0x45c431ae12LL
//#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
//#define RF24ADDRESS 0x45c431ae96LL
//#define RF24ADDRESS 0x45c431aeabLL
//#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x71


#define LED_PIN 8

BayRF24 client=BayRF24(9,10);


void blink(uint8_t times){
  for(uint8_t i=0;i<times;i++){
    digitalWrite(LED_PIN,HIGH);
    delay(200);
    digitalWrite(LED_PIN,LOW);
    delay(300);
  }
}

void setup(void){
  pinMode(LED_PIN,OUTPUT);
  client.init(RF24ADDRESS,RF24CHANNEL);
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis());     
   if(client.sendPayload())
     blink(2);
   else
     blink(1);
  delay(1500);
   
}

