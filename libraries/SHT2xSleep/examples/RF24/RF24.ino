/****************************************************************
 *
 ***************************************************************/

#include <Wire.h>
#include <Sleep.h>
#include <SHT2xSleep.h>
#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>

volatile uint8_t ticks;
ISR(TIMER2_OVF_vect){
  ticks++;
}
BayRF24 client=BayRF24(9,10);


void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  Wire.begin();
  client.init(0x45c431ae48LL);
  startFrameReadVoltage();
}

void loop()
{
  if((ticks%64)==0){ //every four seconds
    unsigned long t=millis();
    float hum=SHT2x.GetHumidity();
    float temp=SHT2x.GetTemperature();
    client.addChannelValue(temp);
    client.addChannelValue(hum);
    client.sendPayload();
    //Read battery voltage _after_ long uptime!!!
    startFrameReadVoltage();
  }  
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}


/* Read battery voltage */
void startFrameReadVoltage(void){
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis());
  analogReference(INTERNAL);
  pinMode(A3,OUTPUT);
  digitalWrite(A3,HIGH);
  client.addChannelValue(1.1*320/100/1023*analogRead(A0));
  digitalWrite(A3,LOW);
  pinMode(A3,INPUT);
  analogReference(DEFAULT);
}
