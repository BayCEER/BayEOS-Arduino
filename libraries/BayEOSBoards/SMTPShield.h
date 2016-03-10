/*
 * Just a bunch of variables and functions to
 * handle measurements on SMTPShield
 *
 * expects to have a bayeosClient called "client"
 * and a RTC called "myRTC"
  channel 1 - CH-Status (0 sleeping - 1 charging - 2 done - 3 error)
  channel 2 U_LiPo (V)
  channel 3+4 - uptime/cpu-time (s)
  channel 5 - RTC-Temperature (°C)
  channel 6-11 - EC5 (%)
  channel 12 Kippwaage (count)
  channel 13-14 Dallas (Currently one sensor per bus)
 */

#define DALLAS1_POWER  A0
#define DALLAS1_DATA A1
#define DALLAS2_POWER A2
#define DALLAS2_DATA A3


float tmp_float ;
const uint8_t echo_power_pin[6]={3,4,6,7,8,9};
volatile uint8_t kippevent=0;
volatile uint8_t wdcount=0;

uint8_t count_kippevent=0;
uint8_t kippcount=0;
unsigned long start_time;
unsigned long last_data;

DS18B20 ds1=DS18B20(DALLAS1_DATA,0,1);
DS18B20 ds2=DS18B20(DALLAS2_DATA,0,1);
MCP342x mcp = MCP342x();

void handleKippevent(){
	  if(count_kippevent){
	    kippcount++;
	    count_kippevent=0;
	    kippevent=0;
	  }
	 if(kippevent && ! count_kippevent){
   	   count_kippevent=1;
   	   delay(400);
	  }
}

void measure(void){
      last_data=myRTC.now().get();
 	//send buffered frames
	//only one each time



	// Dallas
    myRTC.convertTemperature();
    digitalWrite(DALLAS1_POWER,HIGH);
    digitalWrite(DALLAS2_POWER,HIGH);
	ds1.t_conversion();
	ds2.t_conversion();
	delay(630);
        //Normally you have to wait 750, but we will do some other stuff meanwhile...
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

	//Read ECHO5
	for(uint8_t i=0;i<6;i++){
		//Serial.println(i);
   	  pinMode(echo_power_pin[i],OUTPUT);
	  digitalWrite(echo_power_pin[i],HIGH);
      delay(10);
 	  mcp.setConf((i>=4)+3,1,(i%4),1,0,0);
	  delay(10);
	  tmp_float = mcp.getData((i>=4)+3)*119-40;
	  digitalWrite(echo_power_pin[i],LOW);
      client.addToPayload(tmp_float);  // Float
   	  pinMode(echo_power_pin[i],INPUT);
	}
 	tmp_float=kippcount;
 	kippcount=0;
 	client.addToPayload(tmp_float);

 	if(ds1.readChannel(1,&tmp_float))
 		tmp_float=-255;
  	client.addToPayload(tmp_float);
    digitalWrite(DALLAS1_POWER,LOW);
 	if(ds2.readChannel(1,&tmp_float))
 		tmp_float=-255;
  	client.addToPayload(tmp_float);
    digitalWrite(DALLAS2_POWER,LOW);

}

//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
// You have to set this! Otherwise it will call reset!
ISR(WDT_vect) {
	wdcount++;
	if(wdcount>240){
        asm volatile (" jmp 0"); //restart programm
    }
}

void kippwaage(){
  kippevent=1;
}

void initShield(void){
	  attachInterrupt(0, kippwaage, FALLING);
	  Wire.begin();
	  myRTC.begin();

	  pinMode(DALLAS1_POWER, OUTPUT);
	  digitalWrite(DALLAS1_POWER,HIGH);
	  pinMode(DALLAS2_POWER, OUTPUT);
	  digitalWrite(DALLAS2_POWER,HIGH);
	  delay(50);
	  ds1.setAllAddr();
	  ds2.setAllAddr();
      start_time=myRTC.now().get();

}


