#include <Wire.h>
#include <MCP342x.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>

#define D_IR 2
#define D_B A3
#define D_A A2


#include <BaySerial.h>
#include <BayEOSLogger.h>

BaySerial client(Serial);
BayEOSLogger myLogger;
#define BAUD_RATE 38400
#define CONNECTED_PIN 9
uint8_t connected = 0;
MCP342x mcp342x = MCP342x();
SPIFlash flash(8); //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer; //BayEOS Buffer

#define TICKS_PER_SECOND 4
volatile uint8_t ir_on=0;
#define WITH_TIMER2_ISR_TASK 1
inline void timer2_isr_task(void){
  if(ir_on) digitalWrite(D_IR,!digitalRead(D_IR));
  else digitalWrite(D_IR,1);
}

#include <LowCurrentBoard.h>




const byte channel =1;
const uint8_t rate = 2; //0-3: 12bit ... 18bit

void setup() {
  pinMode(D_IR, OUTPUT);
  digitalWrite(D_IR,1);
  pinMode(D_A, OUTPUT);
  digitalWrite(D_A,0);
  pinMode(D_B, OUTPUT);
  digitalWrite(D_B,0);
  Serial.begin(9600);
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  initLCB(); //init time2
  ir_on=1;
  mcp342x.reset();
  // put in continuous mode
  mcp342x.setConf(0,0,channel,1,rate,0);

}

uint16_t count=0;
unsigned long last_out;
float min=4;
float max=-4;

float mean=0;
uint8_t ch=0;
void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  float data;
  //read until we get a non NaN
  while(isnan(data = mcp342x.getData())){
    delay(1);
  }
  if(data<min) min=data;
  if(data>max) max=data;
  if((millis()-last_out)>2000){
    count++;
    
    uint16_t t=millis()-last_out;
    last_out=millis();
    Serial.print(ch);
    Serial.print("\t");
    Serial.print(min*1000);
    Serial.print("\t");
    Serial.print(max*1000);
    Serial.print("\t");
    Serial.print((max-min)*1000);
    Serial.print("\n");

    if(count>3){
      mean+=(max-min);
    }
    
    if(count>10){
      Serial.print("Mean: ");
      Serial.println(1000*mean/(count-3));
      Serial.println();
      mean=0;
      ch++;
      count=0;
    }
    if(ch==3){ 
      ch=0;
      Serial.println();   

    }
    digitalWrite(D_A, ch&0x1);
    digitalWrite(D_B, ch&0x2); 
    while(isnan(data = mcp342x.getData())){
      delay(1);
    }

    min=+4;
    max=-4;
  }

}


