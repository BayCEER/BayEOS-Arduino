#include <BayEOS.h>
#include <Ethernet.h>
#include <SPI.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPEth.h>


//Please enter a valid Mac and IP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
byte ip[] = { 132, 180, 112, 17 };

BayEth client;


void setup(void){
  pinMode(10,OUTPUT);
  Serial.begin(9600);
  Ethernet.begin(mac,ip);
  //Ethernet.begin(mac);

  client.readConfigFromStringPGM(
    PSTR("192.168.0.1|80||admin|xbee|TestEth|||||")
  );
  //client._urlencode=1;
  Serial.println("Starting");
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(73.43); 
   client.addChannelValue(3.18); 
   client.addChannelValue(millis()/1000); 
   uint8_t res=client.sendPayload();
   Serial.print("res=");
   Serial.println(res);
   client.sendMessage("Just a message!");
   delay(5000);
   
}

