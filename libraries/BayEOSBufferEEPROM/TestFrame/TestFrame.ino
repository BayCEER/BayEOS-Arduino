#include <BayEOSBuffer.h>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <BayEOS.h>
#include <BayDebug.h>


BayDebug client=BayDebug(); 
BayEOSBufferEEPROM myBuffer;
unsigned long last_data=10000;
unsigned long last_buffered_data;


void setup(void){
  client.begin(9600,1);
  Serial.println("Starting...");
  delay(10);
  myBuffer.init(0x50,65536L);
  client.setBuffer(myBuffer);
}

void loop(void){

  //Resend buffered frames
  //one every second
  //sending to frequently my make xbee operationable (channel overload)
  if((millis()-last_buffered_data)>1000){
  	  client.sendFromBuffer();
 	  last_buffered_data=millis();
 
  //Uncomment to get information about the buffer pointer positions
  /*     
         Serial.print("Read-Pos: ");
         Serial.print(myBuffer.readPos());
         Serial.print(" - Write-Pos: ");
         Serial.println(myBuffer.writePos());
   */      
  }

  if((millis()-last_data)>5000){
  	  last_data=millis();
 	  
 	  //Construct DataFrame
      client.startDataFrame(BayEOS_Float32le);
      client.addChannelValue(millis()/1000);
      //client.sendOrBuffer();
      client.writeToBuffer();
      
      //Construct Message
      client.startFrame(BayEOS_Message);
      client.addToPayload("Just a message ;-)");
      //client.sendOrBuffer();
      client.writeToBuffer();
  }                                                                                                                                               

}
