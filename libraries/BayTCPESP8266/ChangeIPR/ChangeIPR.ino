#include <BayTCPESP8266.h>

BayESP8266 client(Serial,7);

void blink(uint8_t times){
  pinMode(LED_BUILTIN,OUTPUT);
  while(times){
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    times--;
  }
}

void setup(void){
  uint8_t res=client.changeIPR(38400);
  if(res){
    blink(2); //Failed to set baud rate
  } else {
    blink(4); //new baud rate set
  }
}

void loop(void){
  
}

