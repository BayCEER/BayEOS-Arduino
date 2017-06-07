#include <Sleep.h>

unsigned long freq[256];

volatile uint8_t measure;
volatile unsigned long current_micros, last_micros;
ISR(TIMER2_OVF_vect) {
  last_micros=current_micros;
  current_micros=micros();
  if(measure){
    freq[OSCCAL]=(current_micros-last_micros)*8*128;
    OSCCAL++;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  Serial.println("Starting ...");
  Serial.println(OSCCAL);
  Serial.println("------------");

  uint8_t osc=OSCCAL;

  
  Sleep.setupTimer2(1); //init timer2 to 0,0078125sec
  delay(200);
  
  OSCCAL=0;
  measure=1;
  while(OSCCAL<255){
    
  }
  measure=0;
  OSCCAL=osc;

  for(uint8_t i=0;i<255;i++){
    Serial.print(i);
    Serial.print("\t");
    Serial.println(freq[i]);
  }

  for(uint8_t i=0;i<255;i++){
    Serial.print(i);
    Serial.print("\t");
    delay(50);
    OSCCAL=i;
    Serial.print(freq[i]);
    delay(100);
    OSCCAL=osc;
    Serial.println();
  }
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
