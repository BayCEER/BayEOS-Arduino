#include <MCP_MPLEX.h>
#include <Wire.h>
#include <MCP342x.h>
#include <math.h>

const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 2; //0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);

//Configure your resistors on the board!
const uint16_t R[]={ 19877, 19975, 20028, 19966,
                     20015, 19926, 19898, 19912 };

float ntc10_R2T(float r){
  float log_r=log(r);
  return 440.61073-75.69303*log_r+
         4.20199*log_r*log_r-0.09586*log_r*log_r*log_r;
}


void setup()
{
  Serial.begin(9600);
  Wire.begin();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(MPLEX_POWER_PIN,OUTPUT);
  pinMode(5,OUTPUT);
}

void loop(){
  digitalWrite(5,HIGH);
  digitalWrite(MPLEX_POWER_PIN,HIGH);
  for(uint8_t ch=0; ch<8;ch++){
    mplex_set_channel(ch);   
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    float I = mcp342x.getData()/(float) R[ch];

    mcp342x.runADC(1);
    delay(mcp342x.getADCTime());
    float R_mess=mcp342x.getData()/I;
    Serial.print(ntc10_R2T(R_mess));
    Serial.print("\t");
  }
  Serial.println();
  digitalWrite(5,LOW);
  digitalWrite(MPLEX_POWER_PIN,LOW);
  delay(2000);
}

