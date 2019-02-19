/******************************

Example for continuous Mode

the Sample/s depend on the resolution:

Resolution | Samples per Second
    12 bit | 256
    14 bit | 64
    16 bit | 16
    18 bit | 4 
   
*******************************/



#include <MCP342x.h>

const byte addr = 0;
const byte channel =0;
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t gain = 0; //0-3: 1x ... 8x
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);



void setup()
{
  Serial.begin(9600);
  mcp342x.reset();
  // put in continuous mode
  mcp342x.setConf(addr,0,channel,1,rate,gain);
  Serial.println("ready");

}


uint16_t count=0;
unsigned long last_out;
float min=4;
float max=-4;
void loop()
{
  float data;
  //read until we get a non NaN
  while(isnan(data = mcp342x.getData())){
    delay(1);
  }
  if(data<min) min=data;
  if(data>max) max=data;
  count++;
  if((millis()-last_out)>2000){
    uint16_t t=millis()-last_out;
    last_out=millis();
    Serial.print(t);
    Serial.print("\t");
    Serial.print(min*1000);
    Serial.print("\t");
    Serial.print(data*1000);
    Serial.print("\t");
    Serial.print(max*1000);
    Serial.print("\t");
    Serial.print(count);
    Serial.print("\t");
    Serial.println(1000.0*count/t);
    count=0;
    min=+4;
    max=-4;
  }

}


