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
  channel 6 D1 (V)
  channel 7 D2 (V)
  channel 8 D3 (V)
  channel 9 D4 (V)

 */

#define UPSTEPPER_EN_PIN A3

#define MCP_ADDR_OFFSET 1

#define XBEE_SLEEP_PIN 5
float tmp_float ;

unsigned long start_time;
unsigned long last_data;
volatile uint8_t wdcount=0; //set 0 in loop otherwise board will call a reset!!

MCP342x mcp = MCP342x();

#ifndef HMPBOX
#define HMPBOX 0
#endif

void measure(void){
    last_data=myRTC.now().get();
 	//send buffered frames
	//only one each time
    myRTC.convertTemperature();
    digitalWrite(UPSTEPPER_EN_PIN,HIGH);
    delay(50);
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

    if(HMPBOX) delay(2000);

    //Read MCP
    for(uint8_t i=0;i<4;i++){
    	mcp.setConf(MCP_ADDR_OFFSET,1,i,0,0,0);
    	delay(10);
    	tmp_float = (i<2 && HMPBOX?1:2)*mcp.getData(MCP_ADDR_OFFSET);
    	client.addToPayload(tmp_float);  // Float
    }
    digitalWrite(UPSTEPPER_EN_PIN,LOW);

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


void initShield(void){
	  Wire.begin();
	  myRTC.begin();
	  pinMode(UPSTEPPER_EN_PIN, OUTPUT);
	  digitalWrite(UPSTEPPER_EN_PIN,LOW);
      start_time=myRTC.now().get();
}

void swap(float *x,float *y)
{
    float temp;
    temp = *x;
    *x = *y;
    *y = temp;
}


void buble_sort(float a[],const int size)
{
    int i,j;
    for(i=0; i<(size-1); i++)
    {
        for(j=0; j<(size-(i+1)); j++)
        {
            {
                if(a[j] > a[j+1])
                    swap(&a[j],&a[j+1]);
            }

        }
    }
}

