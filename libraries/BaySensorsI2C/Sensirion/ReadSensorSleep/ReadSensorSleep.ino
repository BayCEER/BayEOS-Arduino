#include <Sensirion.h>
#include <Sleep.h>
ISR(TIMER2_OVF_vect){
}

Sensirion sht2 = Sensirion(4, 6, 0x40);
float temperature;
float humidity;
int ret;


void setup()
{
  Sleep.setupTimer2(1); //init timer2 to 0,0078125sec
  Serial.begin(9600);
  Serial.println("Starting");
  delay(20);
}

void loop()
{
  long start=millis();
  ret = sht2.measureSleep(&temperature, &humidity);
  start=millis()-start;
  if (! ret) 
  { 
    Serial.print("CPU-Time = "); 
    Serial.print(millis());
    Serial.print("ms, "); 
    Serial.print("Call = "); 
    Serial.print(start);
    Serial.print("ms, "); 
    Serial.print("Temperature = ");   
    Serial.print(temperature);
    Serial.print("oC, Humidity = ");  
    Serial.print(humidity);
    Serial.println("%"); 
  } else {
    Serial.print("Error: ");
    Serial.println(ret);
  }
  delay(100); //Normal delay to let Serial transmit data
  //Sleep for 200x0,0078125sec
  for(uint8_t i=0;i<200;i++){
    Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE);
  }
   
}
