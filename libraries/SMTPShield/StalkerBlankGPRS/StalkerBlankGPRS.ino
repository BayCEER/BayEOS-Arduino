/*

Blank Stalker Sketch...

   Stefan Holzheu
   30.01.2013
   
  channel 1+2 - CH-Status+U_LiPo
  channel 3+4 - uptime/cpu-time
  channel 5 - RTC-Temperature   
*/
#include <EEPROM.h>
#include <Wire.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <Base64.h>
#include <BayGPRS.h>
#include <BayDebug.h>
#include <Sleep.h>

#define SAMPLING_INT 30
#define SENDING_INT 300

#define MIN_VOLTAGE_OFF 3.9
#define MIN_VOLTAGE_ON 4.05


DS3231 myRTC; //Seduino 2.2


BayGPRS client; //Sleep-Pin - Wakeuptime, timeout

//BayDebug client=BayDebug(); 
BayEOSBufferSDFat myBuffer;

float tmp_float ;
unsigned long start_time;
unsigned long last_data;
unsigned long last_send;
uint8_t status, lastRSSI;

//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
// You have to set this! Otherwise it will call reset!
volatile uint8_t wdcount=0;
ISR(WDT_vect) {
	wdcount++;
	if(wdcount>240){
        asm volatile (" jmp 0"); //restart programm
    }
}


void setup() {
  status=1; //startup!
  //Set 4 for EthernetShield, 10 for Stalker
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
  }
  client.readConfigFromFile("gprs.txt");
  
//  client.readConfigFromEEPROM(); //Note you have to use an other Sketch to write config!!
  Sleep.setupWatchdog(6); //init watchdog timer to 1 sec
//  client.printConfig(); 
  client.begin(38400); 
//  client.connect();
  
  myBuffer=BayEOSBufferSDFat(200000000,1);
  myBuffer.setRTC(myRTC,false); //Nutze RTC jedoch relativ!

  client.setBuffer(myBuffer,100); //max skip=100!!

  Wire.begin();
  myRTC.begin();
  start_time=myRTC.now().get();


}

void loop() {
  if((myRTC.now().get()-last_data)>=SAMPLING_INT){


      last_data=myRTC.now().get();
    myRTC.convertTemperature();
    client.startDataFrame(BayEOS_Float32le);
    client.addToPayload((uint8_t) 0);  //Offset
	// Charge
    analogReference(INTERNAL);
	tmp_float=analogRead(A6);
	/*
	 * >900 sleeping
	 * >550 charging
	 * >350 done
	 * <350 error
	*/
	if(tmp_float>900) tmp_float=0;
	else if(tmp_float>550) tmp_float=1;
	else if(tmp_float>350) tmp_float=2;
	else tmp_float=3;

    client.addToPayload(tmp_float);  // Float
	tmp_float=(1.1 / 1024)*analogRead(A7)*(10+2)/2; //Voltage
	/*
	 *  voltage = tmp_float * (1.1 / 1024)* (10+2)/2;  //Voltage divider
	 */
    client.addToPayload(tmp_float);  // Float

    client.addToPayload((float) (last_data-start_time));
    client.addToPayload((float) (millis()/1000));
    client.addToPayload((float) myRTC.getTemperature());
    client.addToPayload((float) myBuffer.readPos());
    client.addToPayload((float) myBuffer.writePos());
    client.addToPayload((float) lastRSSI);

    //Send data or write to buffer 
        client.writeToBuffer();	  
 //   client.sendPayload();
  }
  wdcount=0;
  
  if((tmp_float>MIN_VOLTAGE_OFF && status==2) || tmp_float>MIN_VOLTAGE_ON){
    //Power ok!
    if(myBuffer.available() &&
    (((myRTC.now().get()-last_send)>=SENDING_INT || status==1)
     || myBuffer.available()>5000  )) {
      status=2; //connected
      last_send=myRTC.now().get();
      client.sendMultiFromBuffer();
      lastRSSI=client.getRSSI();
    }
  } 
  
  if(tmp_float<MIN_VOLTAGE_OFF && status!=3) {
    client.softSwitch();
    delay(1000);
    if(client.sendATE0()) status=3; //off!
  }

  //sleep until watchdog will wake us up...
  Sleep.sleep();

  
}

