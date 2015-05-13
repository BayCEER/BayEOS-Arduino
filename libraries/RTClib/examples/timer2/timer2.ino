/*
You need to have a 32kHz quarz connected!!!!!

*/
#include <Wire.h>
#include <BayEOSBuffer.h>
#include <RTClib.h>


RTC_Timer2 RTC;


ISR(TIMER2_OVF_vect){
  RTC._seconds += 1; 
}


void setup () {
    Serial.begin(9600);
    //TIMER2 Settings
    TCCR2A = 0x00;
    TCCR2B = 5; //Timer2 overflow every second
    ASSR = (1<<AS2); //Enable asynchronous operation
    TIMSK2 = (1<<TOIE2); //Enable the timer 2 interrupt
    RTC.begin();
    
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
}

void loop () {
    DateTime now = RTC.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since 2000 = ");
    Serial.print(now.get());
    Serial.print("s = ");
    Serial.print(now.get() / 86400L);
    Serial.println("d");
    
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now.get() + 7 * 86400L + 30);
    
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();
    
    Serial.println();
    delay(3000);
}
