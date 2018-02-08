/***********************************************************
Power down the ATMEGA to SLEEP_MODE_PWR_DOWN

ATMEGA will consume about 0.3µA
RF24 about 0.9µA



***********************************************************/
#include <Sleep.h>

#define WITH_RF24 0

#if WITH_RF24
#include <BayEOS.h>
#include <BayRF24.h>
BayRF24 client=BayRF24(9,10);
#define RF24ADDRESS 0x45c431ae12LL
#define RF24CHANNEL 0x72
#endif

volatile boolean int0_flag=1;
void int0_isr(void){
  int0_flag=1;
}

void setup(void)
{
  Serial.begin(9600);
  Serial.println("Starting ...");
#if WITH_RF24
  client.init(RF24ADDRESS,RF24CHANNEL);
#endif
  delay(20);
  digitalWrite(2,HIGH);
  attachInterrupt(digitalPinToInterrupt(2), int0_isr, FALLING);
  
}

void loop(void)
{
  Serial.println("Awake ...");
  delay(100);
  if (int0_flag) {
    int0_flag=0;
    Serial.println("Entering Sleep mode");
    delay(200);
    Sleep.sleep();     // sleep function called here
  }
  
}


