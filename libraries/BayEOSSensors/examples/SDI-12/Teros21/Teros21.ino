
/*
 * Beispeil für Teros21 SDI-12 data bus
 * 
 * Funktioniert mit 3.3V
*/

#include <SDI12.h>

#define SERIAL_BAUD 9600 /*!< The baud rate for the output serial port */
#define DATA_PIN 4        /*!< The pin of the SDI-12 data bus */

/** Define the SDI-12 bus */
SDI12 mySDI12(DATA_PIN);
#define POWER_PIN 3

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial)
    ;

  Serial.println("Opening SDI-12 bus...");
  mySDI12.begin();
  delay(500);  // allow things to settle

  // Power the sensors;
  if (POWER_PIN > 0) {
    Serial.println("Powering up sensors...");
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(200);
  }
}

void loop() {
  mySDI12.sendCommand("?M!"); //Measure
  delay(300);
  while (mySDI12.available()) {  
    //Serial.write(mySDI12.read());
    mySDI12.read();
  }
  
  mySDI12.sendCommand("?D0!"); //Read Out Data
  delay(300); 
  // wait a while for a response
  char buffer[20];
  uint8_t i=0;
  while (mySDI12.available()) {  // write the response to the screen
    buffer[i]=mySDI12.read();
    i++;
  }
  buffer[i]=0;
  char* p;
  float hum=strtod(buffer+1,&p);
  float temp=strtod(p,&p);
  //Serial.println(buffer);
  Serial.print(hum);
  Serial.print('\t');
  Serial.println(temp);
  
  delay(3000);  // print again in three seconds
}
