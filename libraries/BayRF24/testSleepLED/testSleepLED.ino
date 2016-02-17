#include <Sleep.h>
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
#define RF24CHANNEL 0x61
#define LED_PIN 5

volatile uint8_t ticks;
volatile uint8_t led_blink=0;
volatile uint8_t led_on=0;
SIGNAL(TIMER2_OVF_vect){
  ticks++;
  if((led_blink>0) && ((ticks%4)==0)){
    digitalWrite(LED_PIN,led_on==0);
    led_on=(led_on==0);
    led_blink--;
  }
  
}
BayRF24 client=BayRF24(9,10);
float voltage;

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  pinMode(LED_PIN,OUTPUT);
  led_blink=6;
  client.init(RF24ADDRESS,RF24CHANNEL);
  readVoltage();
  delay(1000);
}

void loop()
{
  if((ticks%64)==0){ //every four seconds
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    client.addChannelValue(voltage);
    if(client.sendPayload()){
      led_blink=4;
    } else led_blink=2;
    readVoltage(); //after sending!
  }  
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}

void readVoltage(void){
    /* Read battery voltage */
    analogReference(INTERNAL);
    pinMode(A3,OUTPUT);
    digitalWrite(A3,HIGH);
    voltage=1.1*320/100/1023*analogRead(A0);
    digitalWrite(A3,LOW);
    pinMode(A3,INPUT);
    analogReference(DEFAULT);  
}
