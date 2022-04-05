#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);

#define BAUD_RATE 38400

uint8_t buffer[256];
unsigned long max_length, pos;
void setup(void) {
  Serial.begin(BAUD_RATE);
  flash.begin();
  max_length = flash.getCapacity();
  Serial.println("ready");
  if(flash.eraseSector(max_length-4096)){
    Serial.println("Erase last sector ok");
  } else {
    Serial.println("Erase last sector failed");

  }
}

void loop(void){

}
