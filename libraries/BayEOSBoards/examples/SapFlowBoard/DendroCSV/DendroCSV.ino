
/*  A modification of Sapflow-Boards for Dendrometer-Measurements

    For dendrometer measurements remove the two
    resistors and replace one by a bridge

    This sketch collects a number (SEND_COUNT) of measurements and
    sends the data via GPRS to a bayeos gateway

    wiring:
    1 - GND
    2 - common PW
    3 - GND
    4 - SIG Sensor1
    5 - GND
    6 - SIG Sensor2

*/



// Divider resistors for battery voltage
#define BAT_DIVIDER (100.0 + 100.0) / 100.0
#define SENSOR1_LENGTH 11
//#define SENSOR2_LENGTH 11
#define SAMPLING_DELAY 500
#define MAX_ONTIME 30 /*MINUTEN*/

#include <Wire.h>
#include <MCP342x.h>
//Configuration
const byte addr = 0;
const uint8_t gain = 0;  //0-3: x1, x2, x4, x8
const uint8_t mode = 0;  //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 3;  //18bit
#define MCP_POWER_PIN A3
#define POWER_PIN 7

MCP342x mcp342x(addr);

#include <Sleep.h>
volatile boolean int0_flag=0;
void int0_isr(void){
  int0_flag=1;
}
unsigned long wakeup_time;

void setup() {
  Wire.begin();
  Serial.begin(38400);
  pinMode(3,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), int0_isr, FALLING);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN,HIGH);
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  int0_flag=0;
  wakeup_time=millis();
}


void loop() {
    float bat_voltage = 3.3 * BAT_DIVIDER / 1023 * analogRead(A7);
    Serial.print(bat_voltage);
    digitalWrite(MCP_POWER_PIN, HIGH);
    delay(2);
    float v1, v2;
#ifdef SENSOR1_LENGTH
    mcp342x.runADC(2);
    delay(mcp342x.getADCTime());
    v1 = mcp342x.getData();
    mcp342x.runADC(3);
    delay(mcp342x.getADCTime());
    v2 = mcp342x.getData();
    float sensor1 = v2 / (v1 + v2) * SENSOR1_LENGTH * 1000;
    Serial.print(";");
    Serial.print(sensor1);
#endif
#ifdef SENSOR2_LENGTH
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    v1 = mcp342x.getData();
    mcp342x.runADC(1);
    delay(mcp342x.getADCTime());
    v2 = mcp342x.getData();
    float sensor2 = v2 / (v1 + v2) * SENSOR2_LENGTH * 1000;
    Serial.print(";");
    Serial.print(sensor2);
#endif
    Serial.println();
    Serial.flush();
    digitalWrite(MCP_POWER_PIN, LOW);
    delay(SAMPLING_DELAY);
    unsigned long uptime=millis()-wakeup_time;
    if(uptime>60000L*MAX_ONTIME ||  int0_flag){
      if(int0_flag){
        delay(500);
        int0_flag=0;
      }
      digitalWrite(POWER_PIN,LOW);
      digitalWrite(LED_BUILTIN,LOW);
      Sleep.sleep();     // sleep function called here
      wakeup_time=millis();
      digitalWrite(POWER_PIN,HIGH);
      digitalWrite(LED_BUILTIN,HIGH);      
      if(int0_flag){
        delay(500);
        int0_flag=0;
      }

    }

}
