#include <DS18B20.h>
DS18B20 ds(6,0,4);


void setup()
{
  Serial.begin(9600);
}

void loop(){
  ds.setAllAddr(); //Search for new or removed sensors
  for(uint8_t r=9;r<=12;r++){
    ds.setResolution(r);
    ds.t_conversion(); //Start T-conversion 
    delay(1000);
    float temp;
    if(! ds.readChannel(1,&temp)){
      Serial.print(r);
      Serial.print(": ");
      Serial.println(temp);
    }
  }
  Serial.println();

}

