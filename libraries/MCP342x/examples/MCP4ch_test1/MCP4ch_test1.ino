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
const uint8_t gain = 3; //0-3: x1, x2, x4, x8
const uint8_t rate = 3; //0-3: 12bit ... 18bit
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
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
  for(uint8_t ch=0; ch<4;ch++){
    if(ch)
      Serial.print("\t");
    Serial.print("C ");
    Serial.print(ch);
      mcp342x.setConf(addr, 1, ch, mode, rate, gain);
    //  some delay is required - (or maybe not, not tested :-()
    delay(300);
    span = mcp342x.getData(addr);
  
    //  convert float to char[]
    dtostrf(span, 10, 6, str_buf);
  
    Serial.print(str_buf);
  }
  Serial.println();

   
  //  do it every n seconds
  delay(2000);
}


