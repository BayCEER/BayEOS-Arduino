#define NTC10FACTOR 1.0
#define PRE_RESISTOR 10000
#define MCPPOWER_PIN A0
#define HEAT_PIN 6
#define HEAT_RATE 255

#define ZERO_COUNT 10
float zero[6];

#include <MCP342x.h>
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 3; //0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);

float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}
void setup()
{
  Serial.begin(9600);
  Serial.println("Starting");
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(MCPPOWER_PIN, OUTPUT);  
  pinMode(HEAT_PIN, OUTPUT);
  analogWrite(HEAT_PIN,0); //shut off

}

uint8_t count=0;
void loop(){
   float span, strom, ntc10r;
  // Measure and send
   digitalWrite(MCPPOWER_PIN, HIGH);
    for (uint8_t ch = 0; ch < 6; ch++) {
      digitalWrite(A1, ch & 0x4);
      digitalWrite(A2, ch & 0x2);
      digitalWrite(A3, ch & 0x1);
      delay(10);
      mcp342x.runADC(0);
      delay(mcp342x.getADCTime());
      span = mcp342x.getData();
      strom = span / PRE_RESISTOR;

      mcp342x.runADC(1);
      delay(mcp342x.getADCTime());
      span = mcp342x.getData();
      ntc10r = span / strom / NTC10FACTOR;
      if(count<ZERO_COUNT){
        zero[ch]+=ntc10_R2T(ntc10r);
        Serial.print(ntc10_R2T(ntc10r));
        
      } else {
        Serial.print(ntc10_R2T(ntc10r)-zero[ch]/ZERO_COUNT);
      }
      Serial.print("\t");
    }
    Serial.println();
    if(count<ZERO_COUNT){
      count++;
    } else {
        analogWrite(HEAT_PIN,HEAT_RATE); //switch on

    }
    digitalWrite(MCPPOWER_PIN, LOW);
    delay(1000);

}

