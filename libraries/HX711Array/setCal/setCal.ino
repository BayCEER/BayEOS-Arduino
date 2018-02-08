#include <HX711_NTC.h>
HX711Array scale;
HX711_NTC cal(scale);

void setup(void)
{
  Serial.begin(9600);
  cal.init(349000, 3087500, 60574); //init Calibration with default ADC for zero and weight
  cal.reset(); //Reset EEPROM calibration data
  cal.printCalData();
  cal.add(21,349064,3087559);
  cal.add(0,350809,3092297);
  cal.printCalData();
}

void loop(void){}
