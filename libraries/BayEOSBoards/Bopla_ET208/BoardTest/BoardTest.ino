#include <BayEOSBufferSPIFlash.h>
#define POWER_DIVIDER ((1000.0 + 413.0) / 413.0)
#include <MCP342x.h>
MCP342x mcp342x(0);
float span;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayDebug.h>
BayDebug client(Serial);

#define ACTION_COUNT 1
#include <LowCurrentBoard.h>

void setup()
{
  client.begin(9600);
  Serial.println("Starting Test");
  myBuffer.init(flash); //This will restore old pointers
  Serial.print("Flash: ");
  Serial.println(flash.getCapacity());
  Serial.flush();
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); //use skip!
  initLCB(); //init time2
  mcp342x.reset();
  mcp342x.storeConf(1, 0); //14bit,0 gain
  mcp342x.runADC(2);
  delayLCB(mcp342x.getADCTime());
  batLCB = mcp342x.getData() * POWER_DIVIDER;
  Serial.print("MCP3424 Voltage: ");
  Serial.println(batLCB);
  Serial.println("Test done");
  Serial.flush();
  startLCB();

}


void loop() {
 
}



