/******************************

   Reads the value from an AD converter MCP342x and displays
   it in the serial monitor

   Created at the University Bayreuth/Bayceer

*******************************/

#include <MCP342x.h>

// the address of the chip - from 0 up to 8 - it should be hardwired with the chip's address pins on the PCB
// for the chip type mcp3422 the address is allways 0;
const byte addr = 0;
const uint8_t gain = 0; // 0-3: x1, x2, x4, x8
const uint8_t rate = 3; // 0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);

float span = 0.0;

void setup()
{
  Serial.begin(9600);
  Serial.print("General Call Reset... ");
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  Serial.println("fertig");
}

void loop()
{

  // Serial.println("**********************************************");
  for (uint8_t ch = 0; ch < 4; ch++)
  {
    if (ch)
      Serial.print("\t");
    Serial.print("C");
    Serial.print(ch);
    Serial.print(" ");
    mcp342x.runADC(ch);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
    Serial.print(span,6);
  }
  Serial.println();

  //  do it every n seconds
  delay(2000);
}
