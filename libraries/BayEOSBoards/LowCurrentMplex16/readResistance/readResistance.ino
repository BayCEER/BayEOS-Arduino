/*
   Sample Sketch to read resistance using

   MCP, a 16bit Multiplexer and a preresistor


*/
#define PRERESISTOR 10000.0

//When inividual preresistorvalues are given PRERESITOR is ignored
//#define PRERESISTORS {9955.0, 9964.0, 9956.0, 9966.0, 9955.0, 9972.0, 9975.0, 9972.0, 9945.0, 9962.0, 9988.0, 9957.0, 9957.0, 9964.0, 9950.0, 9954.0 }

//Define resolution
const uint8_t rate = 2; //0-3: 12bit ... 18bit

#include <MCP342x.h>
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);

float span = 0.0;
char str_buf[50] = "                            ";


#define MCPPOWER_PIN 6

#ifdef PRERESISTORS
float preresistors[]=PRERESISTORS;
#endif

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
  for (uint8_t ch = 0; ch < 16; ch++) {
    digitalWrite(A1, ch & 0x8);
    digitalWrite(A0, ch & 0x4);
    digitalWrite(A3, ch & 0x2);
    digitalWrite(A2, ch & 0x1);
    delay(1);
    Serial.print(ch);
    Serial.print("\t");
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
 #ifdef PRERESISTORS
    float strom = span / preresistors[ch] * 1000; //current in mA
#else
    float strom = span / PRERESISTOR * 1000; //current in mA
#endif
    dtostrf(span, 10, 6, str_buf);
    Serial.print(str_buf);
    Serial.print("\t");
    dtostrf(strom, 10, 6, str_buf);
    Serial.print(str_buf);
    Serial.print("\t");
    mcp342x.runADC(1);
    delay(mcp342x.getADCTime());
    span = mcp342x.getData();
    dtostrf(span, 10, 6, str_buf);
    Serial.print(str_buf);
    Serial.print("\t");
    dtostrf(span / strom, 10, 6, str_buf); //kOhm
    Serial.println(str_buf);

  }
  digitalWrite(MCPPOWER_PIN, LOW);

  Serial.println();


  //  do it every n seconds
  delay(2000);
}


