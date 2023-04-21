#include <Sleep.h>
#include <HX711Array.h>
uint8_t dout[] = {A3, A2, A1, A0};
uint8_t sck = A4;
long adc[4];
HX711Array scale;
const uint8_t gain=32;
const float vcc=3300000.0; //µV

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  Serial.begin(9600);
  scale.begin(dout, 4, sck); //start HX711Array with 4 ADCs
  scale.set_gain(gain);
  scale.power_down();

}


void loop() {
    scale.power_up();
    scale.read_average(adc,5);
    scale.power_down();
    for(uint8_t i=0;i<4;i++){
      Serial.print(vcc/gain*adc[i]/256/256/256);
      Serial.print("\t");
    }
    Serial.println();
    Serial.flush();
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
}
