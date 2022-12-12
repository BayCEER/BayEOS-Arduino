/*
 * TODO: Continuous Mode does not work :-( //SH 16.02.2018
 * 
 * Reads the value from an AD converter MCP342x and displays it in the serial monitor
 * 
 * Created at the University Bayreuth
 * Bayceer 
*/


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
  Serial.println("Starting...");
  
  //  configure the ADC - Continuous Mode - Channel 0
	//  mcp342x.setConf(addr, B00011100);
  mcp342x.setConf(addr, 1, 0, 1, 3, 0);
}



void loop()
{
  // Wait for conversion to complete
  delay(mcp342x.getADCTime());
  //  read the data from ADC
  span = mcp342x.getData(addr);
  //  convert float to char[]
  dtostrf(span, 12, 5, str_buf);
  
  Serial.println(str_buf);
}


