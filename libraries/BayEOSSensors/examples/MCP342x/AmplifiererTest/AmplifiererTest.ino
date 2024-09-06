/******************************

Tests the amplifier on Channel 0

Put on a constant voltage below 0.256V on Channel 0

The sketch reads the voltage with amplifier set to 8x

Then it changes the amplifier value from 1x, 2x, 4x and 8x and prints out

voltage/voltage_8x

Ideally this should be 1.000 
however we have seen deviation up to 7%
   
*******************************/



#include <MCP342x.h>

// the address of the chip - from 0 up to 8 - it should be hardwired with the chip's address pins on the PCB
// for the chip type mcp3422 the address is allways 0;
const byte addr = 0;
const uint8_t rate = 3; //0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);

float span = 0.0;
char str_buf[50] = "                            ";

void setup()
{
  Serial.begin(9600);
  Serial.print("General Call Reset... ");
  mcp342x.reset();
  Serial.println("fertig");

}



void loop()
{
     mcp342x.storeConf(rate, 3);
    mcp342x.runADC(0);
   delay(mcp342x.getADCTime());
   float ref = mcp342x.getData();

  // Serial.println("**********************************************");
  for (uint8_t g = 0; g < 4; g++) {
    if (g)
      Serial.print("\t");
    Serial.print("G ");
    Serial.print(g);
     mcp342x.storeConf(rate, g);
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();

    //  convert float to char[]
    dtostrf(span/ref, 10, 6, str_buf);

    Serial.print(str_buf);
  }
  Serial.println();


  //  do it every n seconds
  delay(2000);
}


