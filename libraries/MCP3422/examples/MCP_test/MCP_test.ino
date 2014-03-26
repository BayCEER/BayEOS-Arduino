/*Reads the value from an AD converter MCP3422 and displays it in the serial monitor
*
*Created at the University Bayreuth
/Bayceer 
*
*
*/


//#include <WProgram.h>

#include <Wire.h>
#include <MCP3422.h>

//  create an objcet of the class MCP3422
MCP3422 mcp3422 = MCP3422();

float span = 0.0;
char str_buf[50] = "                            ";

void setup()
{
  Serial.begin(9600);
  
  //  configure the ADC
	//  mcp3422.setConf(B00011100);
  mcp3422.setConf(1, 0, 1, 3, 0);
  //  some delay is required - (or maybe not, not tested :-()
  delay(250);
}



void loop()
{
  //  read the data from ADC
  span = mcp3422.getData();
  
  //  convert float to char[]
  dtostrf(span, 12, 5, str_buf);
  
  Serial.println(str_buf);
  
  //  do it every 3 seconds
  delay(3000);
}


