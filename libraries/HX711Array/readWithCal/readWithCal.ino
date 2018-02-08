/*
 * Example sketch for simultaneous reading of four HX711
 * 
 * The library relies on a RTC quarz connected to the MCU
 * 
 */

#include <HX711_NTC.h>
#include <Sleep.h>
#include <EEPROM.h>

uint8_t dout[] = {A5};
uint8_t sck = A4;
HX711Array scale;
HX711_NTC cal(scale);

#define SLOPE_WEIGHT 1467.0
#define ADC_ZERO -70000L
#define ADC_WEIGHT -385000L



volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}


void setup(void)
{
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(20);
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  scale.begin(dout, 1, sck); //start HX711Array with 1 ADCs
  cal.init(ADC_ZERO,ADC_WEIGHT,SLOPE_WEIGHT); //init Calibration with default ADC for zero and weight
  //cal.reset(); //Reset EEPROM calibration data

}

void loop() {
  float temp;

  if(HX711_NTC_mode){
    temp=cal.readNTC();

    cal.cal(temp);
    cal.printCalData();
  }
 // scale.power_up();
  //Serial.print(scale.read_average(20));
  temp=cal.readNTC();
  Serial.print(temp);
  Serial.print("\t");
  delay(100);
  Serial.println(cal.read(temp));
  delay(100);

}


