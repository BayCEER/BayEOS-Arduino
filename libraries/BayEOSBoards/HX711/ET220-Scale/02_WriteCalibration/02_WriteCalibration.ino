/*
   This sketch writes the calibration data to
   ATMEGA328 EEPROM

   For control calibrated reading data is printed to Serial
*/


//BEGIN CALIBRATION DATA
//PASTE HERE the OUTPUT OF THE R-SCRIPT
float sw = 15.034; //Calibration weight of first weight
float t[] = {10.0, 20.0}; //Calibration temperatures
long adc[] = {331519L, 332595L, 1255903L, 1255912L}; //Calibration Values: adc[zero,t0], adc[zero,t1], ...
//END CALIBRATION DATA

#include <HX711Array.h>
#include <NTC.h>
#include <Sleep.h>
uint8_t dout[] = {A3, A2, A1, A0};
uint8_t sck = A4;
HX711Array scale;


#include <DS18B20.h>
DS18B20 ds1(2, 0, 1);
DS18B20 ds2(3, 0, 1);
DS18B20 ds3(4, 0, 1);
DS18B20 ds4(6, 0, 1);
float temp;

Scale4PointCal cal;

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}


void setup(void) {
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  ds1.setAllAddr(); //Search for new or removed sensors
  ds2.setAllAddr();
  ds3.setAllAddr();
  ds4.setAllAddr();
  ds1.setResolution(12);
  ds2.setResolution(12);
  ds3.setResolution(12);
  ds4.setResolution(12);

  Serial.begin(9600);
  Serial.println("Starting ...");
  Serial.flush();
  scale.begin(dout, 4, sck); //start HX711Array with 4 ADCs
  scale.set_gain(128);
  cal.setConf(sw, t, adc);
  cal.saveConf();
  cal.readConf();
  cal.printConf();
  Serial.flush();

}


void loop(void) {
  float t_mean = 0;
  ds1.readChannel(1, &temp);
  t_mean += temp;
  ds2.readChannel(1, &temp);
  t_mean += temp;
  ds3.readChannel(1, &temp);
  t_mean += temp;
  ds4.readChannel(1, &temp);
  t_mean += temp;
  t_mean /= 4;

  long adcs[4];
  uint8_t counts[4];
  scale.power_up();
  while (scale.read_average_with_filter(adcs, 3000, counts) == 0xf0000000) {}
  // scale.read_average(adcs);
  scale.power_down();
  long adc = adcs[0] + adcs[1] + adcs[2] + adcs[3];

  Serial.print("ADC: ");
  Serial.print(adc);
  Serial.print(" (");
  Serial.print(counts[0]+counts[1]+counts[2]+counts[3]);
  Serial.print(")\t");
  Serial.print(t_mean);
  Serial.print("C\t");
  Serial.println(cal.getWeight(adc, t_mean));
  Serial.flush();

}
