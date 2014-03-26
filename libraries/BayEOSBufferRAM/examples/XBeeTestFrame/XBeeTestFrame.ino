#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>


BayXBee client=BayXBee(); 
BayEOSBufferRAM myBuffer;
unsigned long last_data;
unsigned long last_buffered_data;


void setup(void){
  client.begin(38400);
  myBuffer=BayEOSBufferRAM(2000);
  client.setBuffer(myBuffer);
}

void loop(void){

  //Resend buffered frames
  //one every second
  //sending to frequently my make xbee operationable (channel overload)
  if((millis()-last_buffered_data)>1000){
  	  client.sendFromBuffer();
 	  last_buffered_data=millis();
  }

  if((millis()-last_data)>5000){
  	  last_data=millis();
 	  
 	  //Construct DataFrame
      client.startDataFrame(BayEOS_Float32le);
      client.addChannelValue(millis()/1000);
      client.sendOrBuffer();
      
      //Construct Message
      client.startFrame(BayEOS_Message);
      client.addToPayload("Just a message ;-)");
 	  client.sendOrBuffer();
  }                                                                                                                                               

}
