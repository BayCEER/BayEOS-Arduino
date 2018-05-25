
/*
   Sketch for SapFlow Shield

   Run as Logger or RF24
*/


//*********************************************
// User Configruation
//
// Heating Power [W]
#define HEATING_POWER 0.05
// Regulation time step [s]
#define DELTA_T 3
// Resistance of heating coil
#define HEAT_RESISTANCE 30.67
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR_REF 0.5
#define NTC10FACTOR_HEAT 1
//Configure your resistors on the board!
const uint16_t R[] = { 14300, 14300 };

// 470k - 100k Divider + Default Reference
#define BAT_DIVIDER (470.0+100.0)/100.0

//resolution ADC for temperature measurement
const uint8_t rate = 2; //0-3: 12bit ... 18bit


#define RF24ADDRESS 0x172af39a24LL
#define RF24CHANNEL 0x4c
#define CHECKSUM_FRAMES 1
#define SAMPLING_INT 32

//END user configruation
//**********************************************


#include <Wire.h>
#include <MCP342x.h>
#include <math.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BayRF24.h>
unsigned long last_sent;

BayRF24 client(9, 10);


//Configuration
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
#define MCP_POWER_PIN A3
#define HEAT_PIN 6

MCP342x mcp342x(addr);

SPIFlash flash(8); //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer; //BayEOS Buffer
//Configure your resistors on the board!

float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}
#include <LowCurrentBoard.h>


float t_ref, t_heat, bat_voltage;
int heat_rate;
unsigned long last_reg;

float values[9];
int count;

void measure(void) {
  int dt = myRTC.now().get() - last_reg;
  if (dt < DELTA_T) return;
  last_reg = myRTC.now().get();
  digitalWrite(MCP_POWER_PIN, HIGH);
  delay(2);
  if (count == 0) {
    for (uint8_t i = 0; i < 9; i++) {
      values[i] = 0;
    }
  }
  count++;
  mcp342x.runADC(0);
  delay(mcp342x.getADCTime());
  float I = mcp342x.getData() / (float) R[0];
  
  mcp342x.runADC(1);
  delay(mcp342x.getADCTime());
  float R_mess = mcp342x.getData() / I;
  t_ref = ntc10_R2T(R_mess / NTC10FACTOR_REF);

  mcp342x.runADC(2);
  delay(mcp342x.getADCTime());
  I = mcp342x.getData() / (float) R[1];
  
  mcp342x.runADC(3);
  delay(mcp342x.getADCTime());
  R_mess = mcp342x.getData() / I;
  
  t_heat = ntc10_R2T(R_mess / NTC10FACTOR_HEAT);
  digitalWrite(MCP_POWER_PIN, LOW);

  digitalWrite(POWER_PIN, HIGH);
  analogReference(DEFAULT);
  bat_voltage= 3.3 * BAT_DIVIDER / 1023 * analogRead(A7);
  digitalWrite(POWER_PIN, LOW);

    


  values[0] += bat_voltage;
  values[1] += t_ref;
  values[2] += t_heat;
  values[3] += 0;
  values[4] += heat_rate;
  float heat_power=bat_voltage * bat_voltage / HEAT_RESISTANCE * heat_rate / 255.0;
  values[5] += heat_power;

  if(! heat_rate)
    heat_rate=HEAT_RESISTANCE/bat_voltage/bat_voltage*HEATING_POWER*255;
  
  if(values[5]/count>HEATING_POWER && heat_power>HEATING_POWER) heat_rate--;
  else if(values[5]/count<HEATING_POWER && heat_power<HEATING_POWER) heat_rate++;
  
  analogWrite(HEAT_PIN, heat_rate);


  client.startDataFrame(BayEOS_Int16le, CHECKSUM_FRAMES);
  client.addChannelValue(values[0] / count * 1000);
  client.addChannelValue(values[1] / count * 100);
  client.addChannelValue(values[2] / count * 100);
  client.addChannelValue(0); //error
  client.addChannelValue(values[4] / count / 255 *100 * 100); //heat rate
  client.addChannelValue(values[5] / count * 10000); //heat rate in W
#if CHECKSUM_FRAMES
  client.addChecksum();
#endif
}

void setup()
{
  initLCB();
  Wire.begin();
  pinMode(POWER_PIN, OUTPUT);
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  myBuffer.skip(); //This will skip unsent frames
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 100); //use skip!
  startLCB();
}

void loop() {
  measure();
  if (ISSET_ACTION(0) && count > 0) {
    UNSET_ACTION(0);
    sendOrBufferLCB();
    count = 0;
  }
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
}

