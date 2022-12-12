/*
 * Example sketch for simultaneous reading of four HX711
 * 
 * The library relies on a RTC quarz connected to the MCU
 * 
 */

long reads[4];
#include <HX711Array.h>
#include <Sleep.h>

uint8_t dout[] = {A3, A2, A1, A0};
uint8_t sck = A4;
HX711Array scale;

long res[4];
uint8_t counts[4];

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}

void setup(void)
{
  //  pinMode(10, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(20);
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  scale.begin(dout, 4, sck); //start HX711Array with four ADCs
}

void loop() {
  scale.power_up();
  long avg = scale.read_average_with_filter(res,1000,counts);
  scale.power_down();// put the ADC in sleep mode
  
  for(uint8_t i=0;i<4;i++){
    Serial.print("\t");
    Serial.print(res[i]);
    Serial.print(" (");
    Serial.print(counts[i]);
    Serial.print(")");
  }
  Serial.print("\tread average: \t");
  Serial.println(avg);       // print the average of 20 readings from the ADC
  delay(100);

}
