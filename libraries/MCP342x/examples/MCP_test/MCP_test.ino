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
const byte addr = 1;

//  create an objcet of the class MCP342x
MCP342x mcp342x = MCP342x();

float span = 0.0;
char str_buf[50] = "                            ";

void setup()
{
  Serial.begin(9600);
  
  //  configure the ADC
	//  mcp342x.setConf(addr, B00011100);
  mcp342x.setConf(addr, 1, 0, 1, 3, 0);
  //  some delay is required - (or maybe not, not tested :-()
  delay(250);
}



void loop()
{
  //  read the data from ADC
  span = mcp342x.getData(addr);
  
  //  convert float to char[]
  dtostrf(span, 12, 5, str_buf);
  
  Serial.println(str_buf);
  
  //  do it every 3 seconds
  delay(3000);
}


