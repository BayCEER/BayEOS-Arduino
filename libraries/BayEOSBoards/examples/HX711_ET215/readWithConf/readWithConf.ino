/*
   Example Sketch for BayEOS HX711 Boards

   This Sketch can be used to store calibration data to
   ATMEGA328 EEPROM


*/


#include <HX711Array.h>
#include <NTC.h>
#include <Sleep.h>
uint8_t dout[] = {6, 4};
uint8_t sck = 3;
long adc[2];
float temp0, temp1;

HX711Array scale;
NTC_HX711 ntc(scale, A3, 2 * 470000, 3.0); //Adjust resistor values
Scale4PointCal cal0;
Scale4PointCal cal1(28);

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}

#define INIT_CAL 1
#if INIT_CAL
// only for first run
float sw0 = 1467.0; //Calibration weight of first weight cell in gramm
float sw1 = 1463.0; //Calibration weight of second weight in gramm
float t[] = {10.0, 20.0}; //Calibration temperatures
long adc0[] = {331519L, 332595L, 1255903L, 1255912L}; //Calibration Values: adc[zero,t0], adc[zero,t1], ...
long adc1[] = {331519L, 332595L, 1255903L, 1255912L}; //Calibration Values: adc[zero,t0], adc[zero,t1], ...
#endif

volatile uint8_t int0_flag = 0;
void isr_int0(void) {
  int0_flag = 1;
}

void setup(void) {
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), isr_int0, FALLING);

  Serial.begin(9600);
  Serial.println("Starting ...");
  Serial.flush();
  scale.begin(dout, 2, sck); //start HX711Array with 1 ADCs
#if INIT_CAL
  cal0.setConf(sw0, t, adc0);
  cal1.setConf(sw1, t, adc1);
  cal0.saveConf();
  cal1.saveConf();
#endif
  cal0.readConf();
  cal1.readConf();
  cal0.printConf();
  cal1.printConf();
  Serial.flush();
  int0_flag = 1;

}


void loop(void) {
  if (int0_flag) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("Tare ....");
    Serial.flush();
    ntc.readResistance();
    temp0 = ntc.getTemp(0);
    temp1 = ntc.getTemp(1);

    Serial.print(" ... ");
    Serial.flush();
    scale.power_up();
    scale.set_gain(128);
    scale.read_average(adc, 1); //dummy reading
    scale.read_average_with_filter(adc);
    scale.power_down();

    cal0.setTare(adc[0], temp0);
    cal1.setTare(adc[1], temp1);
    Serial.println(" done ");
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(LED_BUILTIN, INPUT);
    int0_flag = 0;
  }

  delay(1000);
  ntc.readResistance();
  temp0 = ntc.getTemp(0);
  temp1 = ntc.getTemp(1);

  scale.power_up();
  scale.set_gain(128);
  scale.read_average(adc, 1); //dummy reading
  scale.read_average_with_filter(adc);
  scale.power_down();

  Serial.print("ADC0: ");
  Serial.print(temp0);
  Serial.print("\t");
  Serial.print(temp1);
  Serial.print("\t");
  Serial.print(adc[0]);
  Serial.print("\t");
  Serial.print(adc[1]);
  Serial.print("\t");
  Serial.print(cal0.getWeight(adc[0], temp0));
  Serial.print("\t");
  Serial.println(cal1.getWeight(adc[1], temp1));
  Serial.flush();

}
