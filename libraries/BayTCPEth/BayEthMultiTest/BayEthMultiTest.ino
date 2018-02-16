/*
Test Multibuffer and URLEncode
*/

#include <BayTCPEth.h>
#include <BayEOSBufferRAM.h>


//Please enter a valid Mac and IP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
byte ip[] = { 132, 180, 112, 17 };

BayEth client;
BayEOSBufferRAM myBuffer;
uint8_t count=0;

void setup(void){
  pinMode(10,OUTPUT);
  Serial.begin(9600);
  Ethernet.begin(mac,ip);
  //Ethernet.begin(mac);

  client.readConfigFromStringPGM(
    PSTR("192.168.0.1|80|gateway/frame/saveFlat|admin|xbee|TestEth$&+,/:;=?@ <>#%{}|~[]`|||||")
  );
  myBuffer=BayEOSBufferRAM(500);
  client.setBuffer(myBuffer);
  //client._urlencode=1;
  Serial.println("Starting");
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(73.43); 
   client.addChannelValue(3.18); 
   client.addChannelValue(millis()/1000); 
   client.writeToBuffer();
   count++;
   if(count>3){
      uint8_t res=client.sendMultiFromBuffer();
     Serial.print("res=");
     Serial.println(res);
     count=0;
   }
   delay(10000);
}

