#include <HX711Array.h>
#include <NTC.h>
#include <Sleep.h>
uint8_t dout[] = {A5};
uint8_t sck = A4;
HX711_4PointCal scale;
NTC_ADC ntc(A1,A2,20000.0,10.0);

#define INIT_CAL 0

#if INIT_CAL
// only for first run 
float sw=1467.0;
float t[]={10.0,20.0};
long adc[]={-68137L,-73803,-378845L,-384611L};
#endif

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}

void setup(void){  
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(20);
  scale.begin(dout, 1, sck); //start HX711Array with 1 ADCs

#if INIT_CAL
  scale.setConf(sw,t,adc);
  scale.saveConf();
#endif
  scale.readConf();
  scale.tare(ntc.getTemp());
  scale.printConf();  

}

void loop(void){
  delay(1000);
  float temp=ntc.getTemp();
  scale.readADC();
  
  Serial.print(temp);
  Serial.print("\t");
  Serial.print(scale.getRaw());
  Serial.print("\t");
  Serial.println(scale.getWeight(temp));
}

