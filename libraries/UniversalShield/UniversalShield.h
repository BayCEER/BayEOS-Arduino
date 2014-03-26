/*
 * Just a bunch of variables and functions to
 * handle measurements on UniversalShield
 *
 * expects to have a bayeosClient called "client"
 * and a RTC called "myRTC"
  channel 1 - CH-Status (0 sleeping - 1 charging - 2 done - 3 error)
  channel 2 U_LiPo (V)
  channel 3+4 - uptime/cpu-time (s)
  channel 5 - RTC-Temperature (°C)
  channel 6 EC5 (%)
  channel 7 HMP Moisture (%)
  channel 8 HMP Temperature (%)
  channel 9 PAR (V)
  channel 10 Kippwaage (count)
  channel 11 Dallas (Currently one sensor per bus)
 */

#define DALLAS1_POWER  6
#define DALLAS1_DATA 5
#define UPSTEPPER_EN_PIN A3
#define ECHO_POWER_PIN 7

#define ADC_CH_ECHO 1
#define ADC_CH_HMP_T 0
#define ADC_CH_HMP_H 3
#define ADC_CH_PAR 2

#define XBEE_SLEEP_PIN 8
float tmp_float ;

volatile uint8_t kippevent=0;
uint8_t last_kippevent=0;
uint8_t count_kippevent=0;
uint8_t kippcount=0;
unsigned long start_time;
unsigned long last_data;

DS18B20 ds1=DS18B20(DALLAS1_DATA,0,1);
MCP342x mcp = MCP342x();

void handleKippevent(){
	 if(kippevent && ! count_kippevent){
	   last_kippevent=myRTC.now().get();
	   count_kippevent=1;
	  }
	  if(count_kippevent && (((uint8_t) myRTC.now().get())-last_kippevent)>1){
	    kippcount++;
	    count_kippevent=0;
	    kippevent=0;
	  }
}

void measure(void){
    last_data=myRTC.now().get();
 	//send buffered frames
	//only one each time


    digitalWrite(UPSTEPPER_EN_PIN,HIGH);
    delay(1700);
	// Dallas
    myRTC.convertTemperature();
    digitalWrite(DALLAS1_POWER,HIGH);
	ds1.t_conversion();
	delay(350);
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
    digitalWrite(ECHO_POWER_PIN,HIGH);
    delay(10);
 	mcp.setConf(3,1,ADC_CH_ECHO,1,0,0);
	delay(10);
	tmp_float = mcp.getData(3)*119-40;
	digitalWrite(ECHO_POWER_PIN,LOW);
    client.addToPayload(tmp_float);  // Float

    //Read HMP
    delay(10); //Wie lange???
 	mcp.setConf(3,1,ADC_CH_HMP_H,1,0,0);
	delay(10);
	tmp_float = mcp.getData(3)*100;
    client.addToPayload(tmp_float);  // Float

 	mcp.setConf(3,1,ADC_CH_HMP_T,1,0,0);
	delay(10);
	tmp_float = mcp.getData(3)*120-40;
    client.addToPayload(tmp_float);  // Float

    digitalWrite(UPSTEPPER_EN_PIN,LOW);

	//Read PAR
	mcp.setConf(3,1,ADC_CH_PAR,1,3,3);
	delay(350);
	tmp_float=mcp.getData(3);
	client.addToPayload(tmp_float);

	tmp_float=kippcount;
 	kippcount=0;
 	client.addToPayload(tmp_float);

 	if(ds1.readChannel(1,&tmp_float))
 		tmp_float=-255;
  	client.addToPayload(tmp_float);
    digitalWrite(DALLAS1_POWER,LOW);
}

//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
// You have to set this! Otherwise it will call reset!
ISR(WDT_vect) {
}

void kippwaage(){
  kippevent=1;
}

void initShield(void){
	  attachInterrupt(0, kippwaage, FALLING);
	  Wire.begin();
	  myRTC.begin();

	  pinMode(DALLAS1_POWER, OUTPUT);
	  pinMode(UPSTEPPER_EN_PIN, OUTPUT);
	  digitalWrite(DALLAS1_POWER,HIGH);
	  delay(50);
	  ds1.setAllAddr();
      start_time=myRTC.now().get();

}


