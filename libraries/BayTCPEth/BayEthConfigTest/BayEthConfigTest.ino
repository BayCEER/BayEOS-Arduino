#include <BayEOS.h>
#include <SdFat.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferSDFat.h>
#include <Ethernet.h>
#include <SPI.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPEth.h>




byte mac[6];
byte ip[4];
byte mask[4];
byte default_gw[4];

BayEth client;


void setup(void){
  pinMode(10,OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting");
  delay(10);
  if (!SD.begin(4)) {
    Serial.println("SD failed");
    return;
  }
  client.readConfigFromFile("ETH.TXT");
  
  memcpy(mac,client.parseMAC(*client.getConfigPointer(BayTCP_CONFIG_MAC)),6);
  memcpy(ip,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_IP)),4);
  memcpy(mask,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_MASK)),4);
  memcpy(default_gw,client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_DEFAULT_GW)),4);
  

  uint8_t i;
  for(i=0;i<6;i++){
    Serial.print(mac[i],HEX);
    Serial.print(':');
  }
  Serial.println();
  for(i=0;i<4;i++){
    Serial.print(ip[i]);
    Serial.print('.');
  }
  Serial.println();
  for(i=0;i<4;i++){
    Serial.print(mask[i]);
    Serial.print('.');
  }
  Serial.println();
  for(i=0;i<4;i++){
    Serial.print(default_gw[i]);
    Serial.print('.');
  }
  Serial.println();

  for(i=0;i<10;i++){
    Serial.print((i+1));
    Serial.print(":");
    Serial.println(*client.getConfigPointer(i));
  }
  

  Ethernet.begin(mac, ip, default_gw, default_gw, mask);
  Serial.println("Setup OK");
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
   //client.sendMessage("Just a message!");
   
    
  delay(5000);
   
}

