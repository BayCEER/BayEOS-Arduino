/*
   Sample Sketch to read resistance using

   MCP, a 16bit Multiplexer and a preresistor


*/
#define PRERESISTOR 14300.0
const uint8_t rate = 3; //0-3: 12bit ... 18bit
#define MCPPOWER_PIN 6

#include <BayEOSBufferSPIFlash.h>
#define POWER_DIVIDER ((100.0 + 100.0) / 100.0)
#include <MCP342x.h>
MCP342x mcp342x(0);
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
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
  pinMode(POWER_PIN,OUTPUT);
  digitalWrite(POWER_PIN,HIGH);
  mcp342x.storeConf(1, 0); //14bit,0 gain
  batLCB = 3.3*analogRead(A7)/1023*POWER_DIVIDER;
  Serial.print("Bat Voltage: ");
  Serial.println(batLCB);
  //digitalWrite(POWER_PIN,LOW);
  
  Serial.println("Test done");
  Serial.flush();
  startLCB();
  pinMode(MCPPOWER_PIN, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

}


void loop() {
  digitalWrite(MCPPOWER_PIN, HIGH);
  for (uint8_t ch = 0; ch < 16; ch++) {
    digitalWrite(A0, ch & 0x8);
    digitalWrite(A1, ch & 0x4);
    digitalWrite(A3, ch & 0x2);
    digitalWrite(A2, ch & 0x1);
    delay(1);
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
    float strom = span / PRERESISTOR; 
    mcp342x.runADC(1);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
    Serial.print(span/strom/1000);
    Serial.print(" ");
  }
  digitalWrite(MCPPOWER_PIN, LOW);
  Serial.println();

 
}
