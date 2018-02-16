/*
This is a example for a simple and cheap logger based on
SPIFlash and TIMER2 RTC
*/


#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>

#define CONNECTED_PIN 5
#define SAMPLING_INT 30

uint8_t connected=0;

//TIME2 RTC - need to have a 32kHz quarz connected!!!!!
RTC_Timer2 myRTC;
ISR(TIMER2_OVF_vect){
  myRTC._seconds += 1; 
}

BaySerial client(Serial); 
SPIFlash flash(10);
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;


//Add your sensor measurements here!
void measure(){
   client.startDataFrame(BayEOS_Int16le);
   client.addChannelValue(millis());
}



void setup() {
  Sleep.setupTimer2(); //init to 1 sec!!
  pinMode(CONNECTED_PIN, INPUT);
  digitalWrite(CONNECTED_PIN,HIGH);
  myBuffer.init(flash,10);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer); 
  //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC,60); //min_sampling_int = 60
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled=1; 
}

void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if(myLogger._logging_disabled && myRTC.now().get()>315360000L)
      myLogger._logging_disabled = 0;
   
   measure();
   myLogger.run();
   

  //sleep until timer2 will wake us up...
  if(! connected){
    myLogger._mode=0;
    Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE);
  }
  
  //check if still connected
  if(connected && digitalRead(CONNECTED_PIN)){
     connected++;
     if(connected>5){
      client.flush();
      client.end();
      connected=0;    
     }
  }
  //Connected pin is pulled to GND
  if(!connected && ! digitalRead(CONNECTED_PIN)){
    connected=1;
    client.begin(38400);
  }
  
}

