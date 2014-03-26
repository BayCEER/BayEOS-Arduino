/*

Blank Stalker Sketch...

   Stefan Holzheu
   30.01.2013
   
  channel 1+2 - CH-Status+U_LiPo
  channel 3+4 - uptime/cpu-time
  channel 5 - RTC-Temperature   
*/

#include <Wire.h>
#include <XBee.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <BayXBee.h>
#include <BayDebug.h>
#include <Sleep.h>

#define XBEE_SLEEP_PIN 5
#define SAMPLING_INT 30
#define UPSTEPPER_EN_PIN A3

DS3231 myRTC; //Seduino 2.2


BayXBee client=BayXBee(XBEE_SLEEP_PIN,15,3000); //Sleep-Pin - Wakeuptime, timeout
//BayDebug client=BayDebug(); 
BayEOSBufferSDFat myBuffer;

float tmp_float ;
unsigned long start_time;
unsigned long last_data;

//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
// You have to set this! Otherwise it will call reset!
ISR(WDT_vect) {
}


void setup() {
  
  Sleep.setupWatchdog(6); //init watchdog timer to 1 sec
  pinMode(XBEE_SLEEP_PIN, OUTPUT);
  pinMode(UPSTEPPER_EN_PIN, OUTPUT);
   
  client.begin(38400); 
  //Set 4 for EthernetShield, 10 for Stalker
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    delay(10000);
    client.sendError("No SD!");
  }
  
  myBuffer=BayEOSBufferSDFat(200000000,1);
  myBuffer.setRTC(myRTC,false); //Nutze RTC jedoch relativ!

  client.setBuffer(myBuffer,100); //max skip=100!!
  Wire.begin();
  myRTC.begin();
  start_time=myRTC.now().get();


}

void loop() {
  if((myRTC.now().get()-last_data)>=SAMPLING_INT){
        digitalWrite(UPSTEPPER_EN_PIN,HIGH);
  delay(4000);
        digitalWrite(UPSTEPPER_EN_PIN,LOW);


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

    //Send data or write to buffer 
        client.writeToBuffer();	  

  }
  client.sendFromBuffer();


  //sleep until watchdog will wake us up...
  Sleep.sleep();

  
}

