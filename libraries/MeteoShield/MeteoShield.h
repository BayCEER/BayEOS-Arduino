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
  channel 6 HMP Moisture (%)
  channel 7 HMP Temperature (%)
  channel 8 PAR1 (V)
  channel 9 PAR2 (V)
  channel 10 Kippwaage (count)
  channel 11 Dallas (Currently one sensor per bus)
 */

#define DALLAS1_POWER  A0
#define DALLAS1_DATA A1
#define UPSTEPPER_EN_PIN A3

#define MCP_ADDR_OFFSET 3
#define ADC_CH_PAR2 1
#define ADC_CH_HMP_T 0
#define ADC_CH_HMP_H 3
#define ADC_CH_PAR1 2

#define XBEE_SLEEP_PIN 5
float tmp_float ;

volatile uint8_t kippevent=0;
volatile uint8_t wdcount=0; //set 0 in loop otherwise board will call a reset!!
//uint8_t last_kippevent=0;
uint8_t count_kippevent=0;
uint8_t kippcount=0;
unsigned long start_time;
unsigned long last_data;

DS18B20 ds1=DS18B20(DALLAS1_DATA,0,1);
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

void measurePAR(void){
    client.startDataFrame(BayEOS_Float32le);
    client.addToPayload((uint8_t) 7);  //Offset 7
	//Read PAR1
	mcp.setConf(MCP_ADDR_OFFSET,1,ADC_CH_PAR1,1,3,3);
	delay(350);
	tmp_float=mcp.getData(MCP_ADDR_OFFSET);
	client.addToPayload(tmp_float);
	//Read PAR2
	mcp.setConf(MCP_ADDR_OFFSET,1,ADC_CH_PAR2,1,3,3);
	delay(350);
	tmp_float=mcp.getData(MCP_ADDR_OFFSET);
	client.addToPayload(tmp_float);
}


void measure(void){
    last_data=myRTC.now().get();
 	//send buffered frames
	//only one each time


    myRTC.convertTemperature();
    digitalWrite(DALLAS1_POWER,HIGH);
        //Normally you have to wait 750, but we will do some other stuff meanwhile...
	ds1.t_conversion();
    digitalWrite(UPSTEPPER_EN_PIN,HIGH);
    delay(1250);
    handleKippevent();
    delay(1250);
    handleKippevent();



    //Start a new data frame
    client.startDataFrame(BayEOS_Float32le);
    client.addToPayload((uint8_t) 0);  //Offset 0
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


    //Read HMP
 	mcp.setConf(MCP_ADDR_OFFSET,1,ADC_CH_HMP_H,1,0,0);
	delay(10);
	tmp_float = mcp.getData(MCP_ADDR_OFFSET)*100;
    client.addToPayload(tmp_float);  // Float

 	mcp.setConf(MCP_ADDR_OFFSET,1,ADC_CH_HMP_T,1,0,0);
	delay(10);
	tmp_float = mcp.getData(MCP_ADDR_OFFSET)*120-40;
    client.addToPayload(tmp_float);  // Float

    digitalWrite(UPSTEPPER_EN_PIN,LOW);

	//Read PAR1
	mcp.setConf(MCP_ADDR_OFFSET,1,ADC_CH_PAR1,1,3,3);
	delay(350);
	tmp_float=mcp.getData(MCP_ADDR_OFFSET);
	client.addToPayload(tmp_float);
	//Read PAR2
	mcp.setConf(MCP_ADDR_OFFSET,1,ADC_CH_PAR2,1,3,3);
	delay(350);
	tmp_float=mcp.getData(MCP_ADDR_OFFSET);
	client.addToPayload(tmp_float);

	tmp_float=kippcount;
 	kippcount=0;
 	client.addToPayload(tmp_float);

 	if(ds1.readChannel(1,&tmp_float))
 		tmp_float=-255;
  	client.addToPayload(tmp_float);
  	//data frame is now complete ...
  	//sending or writing to SD is done in the sketch...


    digitalWrite(DALLAS1_POWER,LOW);
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
	  pinMode(UPSTEPPER_EN_PIN, OUTPUT);
	  digitalWrite(DALLAS1_POWER,HIGH);
	  delay(50);
	  ds1.setAllAddr();
      start_time=myRTC.now().get();

}
