#include <Sleep.h>
#include <MAX11212Board.h>
#include <DS18B20.h>
#include <BaySerial.h>
#include <LCD5110_Basic.h>
extern uint8_t SmallFont[];
//extern uint8_t BigNumbers[];
//int SCK, int MOSI, int DC, int RST, int CS
LCD5110 myGLCD(13,11,12,10,9);
BaySerial client(Serial);
DS18B20 ds(3,0,1);

const byte* new_addr;
uint8_t channel;
float temp;
#define LCB_BAT_ADCPIN A7
#define LCB_BAT_MULTIPLIER 3.3*200/100/1023

#include <LowCurrentBoard.h>

void setup(void) {
  initLCB();
  Serial.begin(9600);
  Serial.println("Starting ...");
  Serial.flush();
  initMAX11212();
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  myGLCD.clrScr();
  myGLCD.print("START",0,8);
  startLCB();

}

void loop(void){
  ds.setAllAddr(); //Search for new or removed sensors
  ds.t_conversion(); //Start T-conversion 
  adc.read(1); //read with calibration
  adc.read(); //read once without calibration
  delayLCB(900);
  float mV=readChannel(0, 30)*1000;
  float wc=4.824e-10*mV*mV*mV-2.278e-6*mV*mV+3.898e-3*mV-2.154; // eqation 2 Teros10 page 14 [m³/m³]
  myGLCD.clrScr();
  char str_buf[15];
  dtostrf(mV, 10, 2, str_buf);
  myGLCD.print("mV:",0,0);
  myGLCD.print(str_buf,24,0);
  dtostrf(wc, 10, 2, str_buf);
  myGLCD.print("%: ",0,8);
  myGLCD.print(str_buf,24,8);

  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  batLCB = LCB_BAT_MULTIPLIER * analogRead(LCB_BAT_ADCPIN);
  digitalWrite(POWER_PIN, LOW);
  pinMode(POWER_PIN, INPUT);

  Serial.print(mV);
  Serial.print("\t");
  Serial.print(wc*100);
  Serial.print("\t");
  if(! ds.readChannel(1,&temp)){
    dtostrf(temp, 10, 2, str_buf);
    myGLCD.print("*C:",0,24);
    myGLCD.print(str_buf,24,24);
      Serial.print(temp);
   Serial.print("\t");
  }
    dtostrf(batLCB, 8, 2, str_buf);
    myGLCD.print("BAT:",0,40);
    myGLCD.print(str_buf,36,40);
      Serial.print(batLCB);
  Serial.println();
  Serial.flush();
  //delay(1000);
  
}

