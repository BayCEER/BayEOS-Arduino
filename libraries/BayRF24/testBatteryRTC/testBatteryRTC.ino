#include <Sleep.h>
#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>

volatile uint8_t ticks;
SIGNAL(TIMER2_OVF_vect){
  ticks++;
}
BayRF24 client=BayRF24(9,10);


void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  client.init(0x45c431ae12LL);
}

void loop()
{
  if((ticks%64)==0){ //every for seconds
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    
    /* Read battery voltage */
    analogReference(INTERNAL);
    pinMode(A3,OUTPUT);
    digitalWrite(A3,HIGH);
    client.addChannelValue(1.1*320/100/1023*analogRead(A0));
    digitalWrite(A3,LOW);
    pinMode(A3,INPUT);
    analogReference(DEFAULT);
    client.sendPayload();
  }  
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}

