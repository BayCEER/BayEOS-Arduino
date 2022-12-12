/*
 * Example sketch for simultaneous reading of four HX711
 * 
 * The library relies on a RTC quarz connected to the MCU
 * 
 */

#define SINGLE_READ 0
long reads[4];
#include <HX711Array.h>
#include <Sleep.h>

uint8_t dout[] = {2, 3, 4, 6};
uint8_t sck = A2;
HX711Array scale;

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
  long single, avg, cpu;
  uint8_t rtc_tics=tics;
  cpu=millis();
  #if SINGLE_READ
  single = scale.read();
  
  #else
  single = scale.read();
  avg = scale.read_average(20);
  #endif
  scale.power_down();// put the ADC in sleep mode
  cpu=millis()-cpu;
  rtc_tics=tics-rtc_tics;
  Serial.print("CPU:\t");
  Serial.print(cpu);
  Serial.print("\tTics: \t");
  Serial.print(rtc_tics);
  Serial.print("\tread: \t");
  Serial.print(single);                 // print a raw reading from the ADC
  #if SINGLE_READ
  for(uint8_t i=0;i<4;i++){
    Serial.print("\t");
    Serial.print(scale.get_single_readings()[i]);
  }
  Serial.println();
  #else
  Serial.print("\tread average: \t");
  Serial.println(avg);       // print the average of 20 readings from the ADC
  #endif
  delay(100);
  for (uint8_t i = 0; i < 64; i++) {
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);    // sleep function called here
  }

}


