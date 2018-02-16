/****************************************************************
 * 
 * Sketch for simple and cheap weather station
 * 
 * with
 * air temperature - air moisture (SHT21)
 * rain gauge
 * soil temperature (Dallas)
 * Allows multiple Dallas on the same bus.
 * Search and delete is done in setup
 *
 * Will store values in I2C-EEPROM
 * 
 * Wiring:
 * Dallas: DATAPIN -> A1, GND-> GND, VCC -> VCC, 4,7k Pullup
 * Rain Gauge: INT0 == D2
 *
 ***************************************************************/
#include <BayEOSBufferEEPROM.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <SHT2xSleep.h>

// 16 ticks per second!
#define WITHDALLAS 0
#define WITHRAINGAUGE 0

#define CONNECTED_PIN 9
#define SAMPLING_INT 30
#define POWER_PIN 7
#define DALLAS_PIN 6
uint8_t connected=0;



BaySerial client(Serial); 
uint8_t i2c_addresses[]={0x50,0x51,0x52,0x53};
BayEOSBufferMultiEEPROM myBuffer;
BayEOSLogger myLogger;


//include some functions for low current board
//expects BayEOS-Client to be called "client"
#include <LowCurrentBoard.h>



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
 

  values[1]+=SHT2x.GetTemperature();
  values[2]+=SHT2x.GetHumidity();
  SHT2x.reset();
  digitalWrite(POWER_PIN,HIGH);
  analogReference(INTERNAL);
  if(digitalRead(CONNECTED_PIN)) //only read if not connected!!
    myLogger._bat=(1.1*320/100/1023*analogRead(A0))*1000;
  values[0]+=((float)myLogger._bat)/1000;
  analogReference(DEFAULT);
  digitalWrite(POWER_PIN,LOW);
  count++; 

  client.startDataFrame(0x41);
  client.addChannelValue(millis(),1);
  for(uint8_t i=0;i<3;i++){ 
    client.addChannelValue(values[i]/count,i+2); 
  }
  #if WITHRAINGAUGE
  client.addChannelValue(rain_count,5);
  #endif
  
  #if WITHDALLAS 
  float temp;
  while(channel=ds.getNextChannel()){
     if(! ds.readChannel(channel,&temp)){
        client.addChannelValue(temp,channel);
      }
  } 
  ds.t_conversion();
  #endif
  
  
}

void setup()
{
  pinMode(CONNECTED_PIN, INPUT);
  digitalWrite(CONNECTED_PIN,HIGH);
  pinMode(POWER_PIN,OUTPUT);
  myBuffer.init(4,i2c_addresses,65536L);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer); 
  //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC,60, 2500); //min_sampling_int = 60
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled=1; 
  Wire.begin();

  initLCB(); //init time2   

  
}

void loop()
{
  //Enable logging if RTC give a time later than 2010-01-01
  if(myLogger._logging_disabled && myRTC.now().get()>315360000L)
      myLogger._logging_disabled = 0;


  #if WITHRAINGAUGE
  handleRainEventLCB();  
  #endif

   
  if(! myLogger._logging_disabled && (myLogger._mode==LOGGER_MODE_LIVE || 
      (myRTC._seconds-last_measurement)>=SAMPLING_INT)){
     last_measurement=myRTC._seconds;
     measure();
   }
   myLogger.run();
   

  if(! connected && myLogger._logging_disabled){
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);
    delayLCB(200);
    digitalWrite(LED_BUILTIN,LOW);
    delayLCB(800);
    pinMode(LED_BUILTIN,INPUT);  
  }
   

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
    adjust_OSCCAL();
    client.begin(38400);
  }
}



