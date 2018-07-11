/*
 * Simple Test-Sketch for BayESP8266
 * 
 * ESP01-Module is on Serial and 
 * CH_PD pin is connected to pin 7
 * 
 *  
 */

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
  //client.changeIPR(38400);
  client.readConfigFromStringPGM(
  PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|TestWLAN|SSID|PASSWORD||"));
  client.begin(38400);
  
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame();
   client.addChannelValue(millis()/1000); 
   /*    
    *     Send data via WLAN
    *     will blink once when all is ok
    *     else result code + 1
    */
   blink(1+client.sendPayload());
   client.powerDown();
   delay(10000);  
}

