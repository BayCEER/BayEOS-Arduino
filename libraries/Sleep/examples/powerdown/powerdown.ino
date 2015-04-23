#include <Sleep.h>

volatile boolean flag_watchdog=1;

void setup(void)
{
//  pinMode(10, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(2);
  Sleep.setupWatchdog(6); //init watchdog timer to 1 sec
}

void loop(void)
{
  Serial.println("Awake ...");
  
  if (flag_watchdog) {
    flag_watchdog=0;
    Serial.println("Entering Sleep mode");
    delay(2);
    Sleep.sleep();     // sleep function called here
  }
  
}


//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
ISR(WDT_vect) {
  flag_watchdog=1;  // set global flag
}
