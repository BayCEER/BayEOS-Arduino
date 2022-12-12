/*
 * Example Sketch for sending data via ESP01
 * 
 * Please program ESP01 with 
 * SerialRouterWiFiManagerWebserver-Sketch from
 * BayEOS-ESP8266-Library
 *  
 *  
 */

#include <BaySerial.h>
#include <BayEOSBufferRAM.h>

void blink(uint8_t times){
  while(times){
    digitalWrite(LED_BUILTIN,HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN,LOW);
    delay(200);
    times--;
  }
  
}

BaySerialESP client(Serial,7);
BayEOSBufferRAM myBuffer;

void setup(void){
  myBuffer = BayEOSBufferRAM(1000);
  client.setBuffer(myBuffer);
  client.begin(38400);
  pinMode(LED_BUILTIN,OUTPUT);
  blink(3);
  client.powerUp();
  //Check if ESP01 is already attached to a WIFI AP
  //IF not ESP01 starts in AP-Mode and can be configured
  //via WIFI (SSID: BayEOS-Serial-Router, PW: bayeos24)
  while(client.isReady()){
    blink(2);
    delay(500);
  }
  client.powerDown();
  delay(1000);
  blink(4);
}

uint8_t count=0;
void loop(void){
  
   client.startDataFrame();
   client.addChannelValue(millis()/1000);     
   client.addChannelValue(analogRead(A0));     
   client.writeToBuffer();
   count++;
   blink(1);
   if(count>10){
     count=0;
     client.powerUp();
     uint8_t res=client.sendMultiFromBuffer();
     blink(res);
     client.powerDown();
   }
   delay(2000);
}
