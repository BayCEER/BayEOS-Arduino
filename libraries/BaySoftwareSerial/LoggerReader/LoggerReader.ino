#include <BayEOS.h>
#include <BayEOSBuffer.h>
#include <SoftwareSerial.h>
#include <BaySoftwareSerial.h>
#include <BayDebug.h>

BaySoftwareSerial rx_client=BaySoftwareSerial(8,9);
BayDebug client;

DateTime now;


void setup(void){
   rx_client.begin(9600);
   client.begin(38400);
   Serial.println("Receiver started...");
}

void loop(void){
  if(Serial.available()){
  	switch(Serial.read()){
  		case 'a':
  		rx_client.startCommand(BayEOS_SetName);
  		rx_client.addToPayload("TEST");
  		break;
  		case 'b':
   		rx_client.startCommand(BayEOS_GetName);
  		break;
  		case 'c':
   		rx_client.startCommand(BayEOS_SetTime);
   		now=DateTime(__DATE__, __TIME__);
  		rx_client.addToPayload((unsigned long) now.get());
  		break;
  		case 'd':
   		rx_client.startCommand(BayEOS_GetTime);
  		break;
  		case 'e':
   		rx_client.startCommand(BayEOS_StartData);
                rx_client.addToPayload((uint8_t) 0);
  		break;
  		case 'f':
   		rx_client.startCommand(BayEOS_StopData);
                rx_client.addToPayload((uint8_t) 0);
  		break;
  		case 'g':
   		rx_client.startCommand(BayEOS_StartData);
                rx_client.addToPayload((uint8_t) 1);
  		break;
  		case 'h':
   		rx_client.startCommand(BayEOS_StartData);
                rx_client.addToPayload((uint8_t) 2);
  		break;
  		case 'i':
   		rx_client.startCommand(BayEOS_StopData);
                rx_client.addToPayload((uint8_t) 1);
  		break;  		
  		case 'j':
   		rx_client.startCommand(BayEOS_StartLiveData);
  		break;  		
  		case 'k':
   		rx_client.startCommand(BayEOS_StopLiveData);
  		break;  		
  		case 'l':
   		rx_client.startCommand(BayEOS_StopLiveData);
  		break;  		
  		case 's':
   		rx_client.sendTXBreak();
  		break;  		
  	}
  	if(rx_client.getPayload(0)==BayEOS_Command){
  	   Serial.print("Sending Command...");
  	   if(rx_client.sendPayload()) Serial.println("failed");
  	   else Serial.println("ok");
  	}
  }

   
  if(rx_client.available() && ! rx_client.readIntoPayload(500)){
    client.startFrame(rx_client.getPayload(0));
    
    for(uint8_t i =1; i<rx_client.getPacketLength();i++){
      client.addToPayload(rx_client.getPayload(i));
    }
    client.sendPayload();
  }
}
