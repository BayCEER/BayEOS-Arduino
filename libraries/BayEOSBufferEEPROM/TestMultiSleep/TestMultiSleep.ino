/*
Test MultiEEPROMBuffer with Sleep

Sleep Current with ATMEGA328, 4 EEPROMS and
RTC Oszillator was

2.2µA


*/

#include <BayEOSBuffer.h>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <BayEOS.h>
#include <BayDebug.h>
#include <Sleep.h>
#include <RTClib.h>

RTC_Timer2 RTC;


ISR(TIMER2_OVF_vect){
  RTC._seconds += 1; 
}



BayDebug client=BayDebug(); 

//define two i2c addresses
uint8_t i2c_addresses[]={0x50,0x51,0x52,0x53};
BayEOSBufferMultiEEPROM myBuffer;
unsigned long last_data=10000;
unsigned long last_buffered_data;


void setup(void){
  client.begin(9600,1);
  Serial.println("Starting...");
  delay(50);
  myBuffer.init(4,i2c_addresses,65536L);
  myBuffer.setRTC(RTC,0); //Nutze RTC relativ!
  myBuffer.reset(); 
  client.setBuffer(myBuffer);
  Sleep.setupTimer2(); //init timer2 to 1 sec
}

void loop(void){

  //Send buffered frames
  //one five second
  if((RTC._seconds-last_buffered_data)>5){
  	  client.sendFromBuffer();
 	  last_buffered_data=RTC._seconds;
          
   //Uncomment to get information about the buffer pointer positions
   /*
         Serial.print("Read-Pos: ");
         Serial.print(myBuffer.readPos());
         Serial.print(" - Write-Pos: ");
         Serial.println(myBuffer.writePos());
    */    
         delay(100);
  }

  if((RTC._seconds-last_data)>20){
  	  last_data=RTC._seconds;
 	  
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
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE);     // sleep function called here
 

}
