/*Reads the value from an AD converter MCP342x and displays it in the serial monitor
*
*Created at the University Bayreuth
/Bayceer 
*
*
*/


//#include <WProgram.h>

#include <Wire.h>
#include <MCP342x.h>

// the address of the chip - from 0 up to 8 - it should be hardwired with the chip's address pins on the PCB
// for the chip type mcp3422 the address is allways 0;
const byte addr = 0;

//  create an objcet of the class MCP342x
MCP342x mcp342x = MCP342x();

float span = 0.0;
char str_buf[50] = "                            ";

void setup()
{
  Serial.begin(9600);
  //  General Call Reset as per Datasheet of the mcp3422/4 
  Serial.print("General Call Reset... ");
  Wire.beginTransmission(B00000000);
  Wire.write(B00000110);
  Wire.endTransmission();
  Serial.println("fertig");
  
}



void loop()
{
 
// Serial.println("**********************************************");
  
  Serial.print("Channel 1 --->  ");
  mcp342x.setConf(addr, B00011100);
  //mcp342x.setConf(addr, 1, 0, 1, 3, 0);
  //  some delay is required - (or maybe not, not tested :-()
  delay(250);
  span = mcp342x.getData(addr);
  
  //  convert float to char[]
  dtostrf(span, 12, 5, str_buf);
  
  Serial.println(str_buf);
  Serial.println();

  Serial.print("Channel 2 --->  ");
  mcp342x.setConf(addr, B00111100);
  //mcp342x.setConf(addr, 1, 0, 1, 3, 0);
  //  some delay is required - (or maybe not, not tested :-()
  delay(250);
  span = mcp342x.getData(addr);
  
  //  convert float to char[]
  dtostrf(span, 12, 5, str_buf);
  
  Serial.println(str_buf);
  Serial.println();

//  --------------------------------

  Serial.print("Channel 3 --->  ");
  mcp342x.setConf(addr, B01011100);
  //mcp342x.setConf(addr, 1, 0, 1, 3, 0);
  //  some delay is required - (or maybe not, not tested :-()
  delay(250);
  span = mcp342x.getData(addr);
  
  //  convert float to char[]
  dtostrf(span, 12, 5, str_buf);
  
  Serial.println(str_buf);
  Serial.println();

//  --------------------------------

  Serial.print("Channel 4 --->  ");
  mcp342x.setConf(addr, B01111100);
  //mcp342x.setConf(addr, 1, 0, 1, 3, 0);
  //  some delay is required - (or maybe not, not tested :-()
  delay(250);
  span = mcp342x.getData(addr);
  
  //  convert float to char[]
  dtostrf(span, 12, 5, str_buf);
  
  Serial.println(str_buf);
  Serial.println();
  
   
  //  do it every n seconds
  delay(2000);
}


