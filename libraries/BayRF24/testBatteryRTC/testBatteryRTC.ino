#include <Sleep.h>
#include <BayRF24.h>

#define POWER_PIN A3

//#define RF24ADDRESS 0x45c431ae12LL
//#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
#define RF24ADDRESS 0x45c431ae96LL
//#define RF24ADDRESS 0x45c431aeabLL
//#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x61

volatile uint8_t ticks;
SIGNAL(TIMER2_OVF_vect){
  ticks++;
}
BayRF24 client=BayRF24(9,10);
float voltage;

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  client.init(RF24ADDRESS,RF24CHANNEL);
  readVoltage();
}

void loop()
{
  if((ticks%64)==0){ //every four seconds
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    client.addChannelValue(voltage);
    client.sendPayload();
    readVoltage(); //after sending!
  }  
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}

void readVoltage(void){
    /* Read battery voltage */
    analogReference(INTERNAL);
    pinMode(POWER_PIN,OUTPUT);
    digitalWrite(POWER_PIN,HIGH);
    voltage=1.1*320/100/1023*analogRead(A0);
    digitalWrite(POWER_PIN,LOW);
    pinMode(POWER_PIN,INPUT);
    analogReference(DEFAULT);  
}
