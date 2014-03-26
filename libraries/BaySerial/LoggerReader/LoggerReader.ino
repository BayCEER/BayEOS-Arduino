/*
 ATTENTION!
 This file will only complile replacing the original 
 arduino-1.0/hardware/arduino/cores/arduino/HardwareSerial.cpp
 arduino-1.0/hardware/arduino/cores/arduino/HardwareSerial.h 
 by the version distributed with the BayEOS arduino libraries
*/

#include <BayEOS.h>
#include <BayEOSBuffer.h>
#include <BaySerial.h>
#include <BayDebug.h>

BaySerial rx_client=BaySerial(Serial2);
BayDebug client;
/*
 * Create a huge ring buffer to store incoming RX Packages while
 * arduino is busy with GPRS...
 */
#define RX_BUFFER_SIZE 1024
unsigned char buffer[RX_BUFFER_SIZE];

uint8_t rp;
unsigned long read_pos;
DateTime now;


void setup(void){
   rx_client.setRxBuffer(buffer, RX_BUFFER_SIZE);
   rx_client.begin(38400);
   client.begin(9600);
   Serial.println("Receiver started...");
}

void loop(void){
  if(Serial.available()){
   		rx_client.sendTXBreak();
  		

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
   		rx_client.startCommand(BayEOS_TimeOfNextFrame);
  		break;
  		case '1':
   		rx_client.startCommand(BayEOS_StartData);
                rx_client.addToPayload((uint8_t) 0);
  		break;
  		case '2':
   		rx_client.startCommand(BayEOS_StartData);
                rx_client.addToPayload((uint8_t) 1);
  		break;
  		case '3':
   		rx_client.startCommand(BayEOS_StopData);
                rx_client.addToPayload((uint8_t) 0);
  		break;
  		case '4':
   		rx_client.startCommand(BayEOS_StopData);
                rx_client.addToPayload((uint8_t) 1);
  		break;
  		case '5':
   		rx_client.startCommand(BayEOS_StopData);
                rx_client.addToPayload((uint8_t) 2);
  		break;
  		case 'j':
   		rx_client.startCommand(BayEOS_StartLiveData);
  		break;  		
  		case 'k':
   		rx_client.startCommand(BayEOS_ModeStop);
  		break; 
   		case 'l':
   		rx_client.startCommand(BayEOS_StartBinaryDump);
                rx_client.addToPayload(read_pos);
  		break;  		
  		case 'm':
   		rx_client.startCommand(BayEOS_StartBinaryDump);
                rx_client.addToPayload((unsigned long) 512);
  		break;  		
  		case 'n':                
   		rx_client.startCommand(BayEOS_StartBinaryDump);
                rx_client.addToPayload((unsigned long) 512);
                 rx_client.addToPayload((unsigned long) 1024);
 		break;  		
    		case 'o':
   		rx_client.startCommand(BayEOS_BufferCommand);
                rx_client.addToPayload((uint8_t) 4); //save last readout
  		break;  		
   		case 'p':
   		rx_client.startCommand(BayEOS_BufferCommand);
                rx_client.addToPayload((uint8_t) 1); //reset
  		break;  		
   		case 'q':
   		rx_client.startCommand(BayEOS_BufferCommand);
                rx_client.addToPayload((uint8_t) 5); //read
                rp=1;
  		break;  		
   		case 'r':
   		rx_client.startCommand(BayEOS_BufferCommand);
                rx_client.addToPayload((uint8_t) 6); //write
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
    if(rp){
       uint8_t* p;
       p =(uint8_t*) &read_pos;
       for(uint8_t i=0;i<4;i++){
         *(p+i)=rx_client.getPayload(i+2);
       } 
       Serial.print("read pos:");
       Serial.println(read_pos);
    }
    rp=0;
  }
}
