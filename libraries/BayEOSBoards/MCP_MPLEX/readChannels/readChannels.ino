#include <MCP_MPLEX.h>
#include <Wire.h>
#include <MCP342x.h>

const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode

MCP342x mcp342x = MCP342x();

//Configure your resistors on the board!
const uint16_t R[]={ 19877, 19975, 20028, 19966,
                     20015, 19926, 19898, 19912 };

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(MPLEX_POWER_PIN,OUTPUT);
  pinMode(5,OUTPUT);
}

void loop(){
  digitalWrite(5,HIGH);
  digitalWrite(MPLEX_POWER_PIN,HIGH);
  for(uint8_t ch=0; ch<8;ch++){
    mplex_set_channel(ch);
    mcp342x.setConf(addr, 1, 0, mode, rate, gain);
    delay(100);
    float I = mcp342x.getData(addr)/(float) R[ch];
    //Serial.print(I*1e6);
    mcp342x.setConf(addr, 1, 1, mode, rate, gain);
    delay(100);
    float R_mess=mcp342x.getData(addr)/I;
    Serial.print(R_mess);
    Serial.print("\t");
  }
  Serial.println();
  digitalWrite(5,LOW);
  digitalWrite(MPLEX_POWER_PIN,LOW);
  delay(2000);
}

