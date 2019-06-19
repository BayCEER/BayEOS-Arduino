#include <Sleep.h>
#include <MAX11212Board.h>
#include <DS18B20.h>

DS18B20 ds1(3,0,3);
DS18B20 ds2(6,3,3);

const byte* new_addr;
uint8_t channel;
float temp;


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
    Serial.print("DS18B20 NR1 Channels: ");
    Serial.print(ds1.getNumberOfChannels(),DEC);
    Serial.print(" - registered ");
    Serial.print(ds1.setAllAddrFromEEPROM(),DEC);
    Serial.print(" Sensors from EEPROM");
    Serial.println();
    Serial.print("DS18B20 NR2 Channels: ");
    Serial.print(ds2.getNumberOfChannels(),DEC);
    Serial.print(" - registered ");
    Serial.print(ds2.setAllAddrFromEEPROM(),DEC);
    Serial.print(" Sensors from EEPROM");
    Serial.println();
  Serial.print("Searching for new Sensors NR1");
  Serial.println();
  while(new_addr=ds1.search()){
    if(channel=ds1.getNextFreeChannel()){
      ds1.addSensor(new_addr,channel);
      Serial.print("Registered new Sensor with ROM ");
      Serial.print(ds1.addr2String(new_addr));
      Serial.print(" on channel ");
      Serial.print(channel,DEC);
      Serial.println();
      } else {
        Serial.print("Failed to add new Sensor with ROM ");
        Serial.print(ds1.addr2String(new_addr));
        Serial.print(": No channel left!!");
        Serial.println();
        break;
      }
  }
  Serial.print("Searching for new Sensors NR2");
  Serial.println();
  while(new_addr=ds2.search()){
    if(channel=ds2.getNextFreeChannel()){
      ds2.addSensor(new_addr,channel);
      Serial.print("Registered new Sensor with ROM ");
      Serial.print(ds2.addr2String(new_addr));
      Serial.print(" on channel ");
      Serial.print(channel,DEC);
      Serial.println();
      } else {
        Serial.print("Failed to add new Sensor with ROM ");
        Serial.print(ds2.addr2String(new_addr));
        Serial.print(": No channel left!!");
        Serial.println();
        break;
      }
  }

}

void loop(void){
  Serial.print("Calibration...");
  Serial.flush();
  adc.read(1); //read with calibration
  adc.read(); //read once without calibration
  Serial.println("done");
  Serial.flush();
  for(uint8_t i=0;i<6;i++){
    Serial.print(readChannel(i,20));
    Serial.print("\t");
    Serial.flush();
  }
  Serial.println();
  Serial.flush();
  ds1.t_conversion();
  ds2.t_conversion();
  delay(1000);
   while(channel=ds1.getNextChannel()){
       if(! ds1.readChannel(channel,&temp)){
         Serial.print("Channel ");
         Serial.print(channel,DEC);
         Serial.print(": ");
         Serial.print(temp);
         Serial.print("C");
         Serial.println();
       }
   }
   while(channel=ds2.getNextChannel()){
       if(! ds2.readChannel(channel,&temp)){
         Serial.print("Channel ");
         Serial.print(channel,DEC);
         Serial.print(": ");
         Serial.print(temp);
         Serial.print("C");
         Serial.println();
       }
   }

  
}

