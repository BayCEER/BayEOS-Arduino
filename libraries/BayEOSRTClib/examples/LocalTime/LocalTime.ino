#include <localtime_DE.h>


void printTime(DateTime& d){
    Serial.print(d.year(), DEC);
    Serial.print('/');
    Serial.print(d.month(), DEC);
    Serial.print('/');
    Serial.print(d.day(), DEC);
    Serial.print(' ');
    Serial.print(d.hour(), DEC);
    Serial.print(':');
    Serial.print(d.minute(), DEC);
    Serial.print(':');
    Serial.print(d.second(), DEC);
    Serial.print(" DOW ");
    Serial.print(d.dayOfWeek(), DEC);
    Serial.println();
}

void setup(void){
  Serial.begin(9600);
//  DateTime utc(2020,4,10,11); //Karfreitag 2020 12.00 Uhr UTC
//  DateTime utc(2020,6,1,11); //Pfingstmontag 2020
  DateTime utc(2020,6,11,11); //Fronleichnam 2020

  DateTime local(utc.get()+getShift(utc));
  Serial.print("UTC-Time:   ");
  printTime(utc);
  Serial.print("Local-Time: ");
  printTime(local);
  Serial.print("Holiday?    ");
  if(isHoliday(local)) Serial.println("Yes");
  else Serial.println("No");
  Serial.print("Daylightsaving? ");
  if(isDaylightSaving(local)) Serial.println("Yes");
  else Serial.println("No");
}

void loop(void){
  
}
