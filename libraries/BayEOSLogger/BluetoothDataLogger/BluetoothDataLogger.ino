#include <BayEOS.h>
#include <BaySerial.h>
#include <BayBluetooth.h>
#include <SdFat.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOSLogger.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <Sleep.h>


#define BAUD_RATE 38400

//R8025 myRTC; //Seduino 2.0
DS3231 myRTC; //Seduino 2.2
BayBluetooth client=BayBluetooth();

BayEOSBufferSDFat  myBuffer;
BayEOSLogger myLogger; 

float tmp_float ;
unsigned long start_time;
unsigned long last_data;

unsigned long last_mode;

ISR(WDT_vect) {
}

void setup() {
  Sleep.setupWatchdog(5); //init watchdog timer to 0.5 sec
  
   Wire.begin();
   myRTC.begin();

   digitalWrite(7, HIGH); // Pull Up 
   client.begin(BAUD_RATE,"BayBluetooth");

   
  //Set 4 for EthernetShield, 10 for Stalker
  if (!SD.begin(10)) {
    return;
  }
  
  //open buffer file in append mode!
  //can be dangerous when buffer wraps arround...
  //make sure, that you manually reset the buffer from time to time...
  myBuffer = BayEOSBufferSDFat(2000000000,1);
 
  //register RTC in buffer
  myBuffer.setRTC(myRTC);

  //register Buffer in bayeos-client
  client.setBuffer(myBuffer);

  //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC);
  start_time=myRTC.now().get();
  delay(1000);
  client.sendMessage("Started");
 }

void loop() {
  // Bluetooth
  if (!digitalRead(7)) 
     client.inquirable();

   //Read sensors
   last_data=myRTC.now().get();
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

    //run logger
 //   client.begin(BAUD_RATE);
 //   delay(200);
    myLogger.run();
    
//    if(last_mode<myRTC.now().get()){
//      last_mode=myRTC.now().get();
//    client.startFrame(BayEOS_Message);
//    client.addToPayload("Mode:");
//    client.addToPayload(myLogger._mode+'0');
//    client.sendPayload();
//    }
/*
    if(! (myLogger._send_data || myLogger._live_data || Serial.available()))
      Sleep.sleep();
      */
}
 
 


