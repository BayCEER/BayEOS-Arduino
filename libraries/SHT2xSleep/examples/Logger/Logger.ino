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
#include <Wire.h>
#include <SHT2xSleep.h>


#define CONNECTED_PIN 4
#define SAMPLING_INT 10
uint8_t connected=0;

//TIME2 RTC - need to have a 32kHz quarz connected!!!!!
volatile uint8_t ticks; //one tick depends on Sleep.setupTimer2
// with 2-> tick is 0,0625sec
// 16 ticks per second!
RTC_Timer2 myRTC;
ISR(TIMER2_OVF_vect){
  ticks++;
  if((ticks % 16)==0) 
    myRTC._seconds += 1; 
}

BaySerial client; 
uint8_t i2c_addresses[]={0x50,0x51,0x52,0x53};
BayEOSBufferMultiEEPROM myBuffer;
BayEOSLogger myLogger;

float values[3];
uint16_t count;
unsigned long last_measurement;

//Add your sensor measurements here!
void measure(){
  if(myLogger._logged_flag){
    myLogger._logged_flag=0;
    count=0;
    for(uint8_t i=0;i<3;i++){ values[i]=0; }
  }
 

  values[1]+=SHT2x.GetHumidity();
  values[2]+=SHT2x.GetTemperature();
  digitalWrite(A3,HIGH);
  analogReference(INTERNAL);
  values[0]+=1.1*320/100/1023*analogRead(A0);
  analogReference(DEFAULT);
  digitalWrite(A3,LOW);
  count++; 

  client.startDataFrame();
  client.addChannelValue(millis());
  for(uint8_t i=0;i<3;i++){ 
    client.addChannelValue(values[i]/count); 
  }
  
}

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  pinMode(CONNECTED_PIN, INPUT);
  digitalWrite(CONNECTED_PIN,HIGH);
  pinMode(A3,OUTPUT);
  myBuffer.init(4,i2c_addresses,65536L);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer); 
  //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC,60); //min_sampling_int = 60
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled=1; 
  Wire.begin();
}

void loop()
{
  //Enable logging if RTC give a time later than 2010-01-01
  if(myLogger._logging_disabled && myRTC.now().get()>315360000L)
      myLogger._logging_disabled = 0;
   
  if(! myLogger._logging_disabled && (myLogger._mode==LOGGER_MODE_LIVE || 
      (myRTC._seconds-last_measurement)>=SAMPLING_INT)){
     last_measurement=myRTC._seconds;
     measure();
   }
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



