/***********************************************************
Power down the ATMEGA to SLEEP_MODE_PWR_DOWN

ATMEGA will consume about 4µA

***********************************************************/
#include <Sleep.h>

volatile boolean flag_watchdog=1;

void setup(void)
{
//  pinMode(10, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(20);
  Sleep.setupWatchdog(8); //init watchdog timer to 4 sec
}

void loop(void)
{
  Serial.println("Awake ...");
  
  if (flag_watchdog) {
    flag_watchdog=0;
    Serial.println("Entering Sleep mode");
    delay(200);
    Sleep.sleep();     // sleep function called here
  }
  
}


//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
ISR(WDT_vect) {
  flag_watchdog=1;  // set global flag
}

