#include <Sleep.h>
#include <MAX11212Board.h>

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}
void setup(void) {
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  Serial.begin(9600);
  Serial.println("Starting ...");
  Serial.flush();
  initMAX11212();
}

void loop(void){
  Serial.print("Calibration...");
  Serial.flush();
  adc.read(1); //read with calibration
  adc.read(); //read once without calibration
  Serial.println("done");
  Serial.flush();
  for(uint8_t i=0;i<6;i++){
    Serial.print(readChannel(i,100));
    Serial.print("\t");
    Serial.flush();
  }
  Serial.println();
  Serial.flush();
}

