/*
 * Sample Sketch to read resistance using
 * 
 * MCP, a 8bit Multiplexer and a preresistor
 * 
 * 
 */
#define PRERESISTOR 14300.0


#include <MCP342x.h>

// the address of the chip - from 0 up to 8 - it should be hardwired with the chip's address pins on the PCB
// for the chip type mcp3422 the address is allways 0;
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 3; //0-3: 12bit ... 18bit
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);

float span = 0.0;
char str_buf[50] = "                            ";


#define MCPPOWER_PIN 6

void setup()
{
  Serial.begin(9600);
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(MCPPOWER_PIN,OUTPUT);
  digitalWrite(MCPPOWER_PIN,HIGH);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
}



void loop()
{
  for(uint8_t ch=0;ch<8;ch++){
    digitalWrite(A1,ch & 0x4);
    digitalWrite(A2,ch & 0x2);
    digitalWrite(A3,ch & 0x1);
    Serial.print(ch);
    Serial.print("\t");
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
    float strom=span/PRERESISTOR*1000; //current in mA
    dtostrf(span, 10, 6, str_buf);
    Serial.print(str_buf);
    Serial.print("\t");
    dtostrf(strom, 10, 6, str_buf);
    Serial.print(str_buf);
    Serial.print("\t");
     mcp342x.runADC(1);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
    dtostrf(span, 10, 6, str_buf);
    Serial.print(str_buf);
    Serial.print("\t");
    dtostrf(span/strom, 10, 6, str_buf);
    Serial.println(str_buf);
    
  }
 
  Serial.println();

   
  //  do it every n seconds
  delay(2000);
}


