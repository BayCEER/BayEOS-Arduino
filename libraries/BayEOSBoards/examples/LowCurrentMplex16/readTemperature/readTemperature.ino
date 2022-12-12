/*
   Sample Sketch to read resistance using

   MCP, a 16bit Multiplexer and a preresistor


*/
#define PRERESISTOR 14300.0
#define NTC10FACTOR 0.5

//Define resolution
const uint8_t rate = 1; //0-3: 12bit ... 18bit

#include <MCP342x.h>
#include <math.h>

float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
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
  pinMode(MCPPOWER_PIN, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
}



void loop()
{
  digitalWrite(MCPPOWER_PIN, HIGH);
  delay(20);  
  for (uint8_t ch = 0; ch < 16; ch++) {
    digitalWrite(A1, ch & 0x8);
    digitalWrite(A0, ch & 0x4);
    digitalWrite(A3, ch & 0x2);
    digitalWrite(A2, ch & 0x1);
    delay(1);
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
    float strom = span / PRERESISTOR; //current in mA
    mcp342x.runADC(1);
    delay(mcp342x.getADCTime());
    float R_mess = mcp342x.getData() / strom / NTC10FACTOR;
    dtostrf(ntc10_R2T(R_mess), 10, 2, str_buf);
    Serial.print(str_buf);
    Serial.print(" ");
  }
  digitalWrite(MCPPOWER_PIN, LOW);

  Serial.println();


  //  do it every n seconds
  delay(1000);
}
