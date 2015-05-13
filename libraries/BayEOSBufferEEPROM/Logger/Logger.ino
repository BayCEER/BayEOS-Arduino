#include <EEPROM.h> 
#include <Wire.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <BayEOS.h>
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

BaySerial client; 
BayEOSBufferEEPROM myBuffer(0x50,64*1024);
BayEOSLogger myLogger;


//Add your sensor measurements here!
void measure(){
   client.startDataFrame(BayEOS_Int16le);
   client.addChannelValue(millis());
}



void setup() {
  Sleep.setupTimer2(); //init to 1 sec!!
  pinMode(CONNECTED_PIN, INPUT);
  Serial.end();
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer); //max skip=100!!
    //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC);
}

void loop() {
  if (myLogger._mode==LOGGER_MODE_LIVE || (myRTC.now().get() - myLogger._last_measurement) 
     >= myLogger._sampling_int){
      measure();
      myLogger.liveData(1000);
	//This will only write Data, when _sampling_interval is reached...
      myLogger.logData();
  }

  
  myLogger.handleCommand();
  myLogger.sendData();
  myLogger.sendBinaryDump();

  //sleep until timer2 will wake us up...
  if(! connected){
    myLogger._mode=0;
    Sleep.sleep();
  }
  
  //check if still connected
  if(connected) 
    connected++;
 
  if(connected>100){
    Serial.flush();
    Serial.end();
    pinMode(1,INPUT);
    pinMode(2,INPUT);
    delay(10);
    connected=0;
  } 

  if(!connected && digitalRead(CONNECTED_PIN)){
    connected=1;
    client.begin(38400);
  }
  
}

