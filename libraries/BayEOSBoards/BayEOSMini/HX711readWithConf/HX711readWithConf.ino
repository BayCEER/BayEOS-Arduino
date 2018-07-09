#include <HX711Array.h>
#include <NTC.h>
#include <Sleep.h>
uint8_t dout[] = {A0};
uint8_t sck = A1;
long adc[1];
float temp;

HX711Array scale;
NTC_HX711 ntc(scale, A3, 2*470000, 5.0); //Adjust resistor values 
Scale4PointCal cal;

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}

#define INIT_CAL 0
#if INIT_CAL
// only for first run
float sw = 1467.0; //Calibration weight of first weight cell in gramm
float t[] = {10.0, 20.0}; //Calibration temperatures
long adc_cal[] = {331519L, 332595L, 1255903L, 1255912L}; //Calibration Values: adc[zero,t0], adc[zero,t1], ...
#endif

volatile uint8_t int0_flag=0;
void isr_int0(void){
  int0_flag=1;
}

void setup(void) {
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  pinMode(2,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2),isr_int0,FALLING);
  
  Serial.begin(9600);
  Serial.println("Starting ...");
  Serial.flush();
  scale.begin(dout, 1, sck); //start HX711Array with 1 ADCs
#if INIT_CAL
  cal.setConf(sw, t, adc_cal);
  cal.saveConf();
#endif
  cal.readConf();
  cal.printConf();  
  Serial.flush();
  int0_flag=1;

}


void loop(void) {
  if(int0_flag){
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);
    Serial.print("Tare ...."); 
    Serial.flush();
    ntc.readResistance();
    temp = ntc.getTemp(0);

    Serial.print(" ... ");
    Serial.flush();
    scale.power_up();
    scale.read_average(adc);
    scale.power_down();

    cal.setTare(adc,temp);
    Serial.println(" done ");
    digitalWrite(LED_BUILTIN,LOW);
    pinMode(LED_BUILTIN,INPUT);
    int0_flag=0;
   }
  
  delay(1000);
  ntc.readResistance();
  temp = ntc.getTemp(0);

  scale.power_up();
  scale.read_average(adc);
  scale.power_down();

  Serial.print("ADC: ");
  Serial.print(temp);
  Serial.print("\t");
  Serial.print(adc[0]);
  Serial.print("\t");
  Serial.print(cal.getWeight(adc[0],temp));
  Serial.print("\n");
  Serial.flush();

}


