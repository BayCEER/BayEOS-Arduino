
#include <Sleep.h>

volatile long seconds = 0;


//The very important 32.686kHz interrupt handler
SIGNAL(TIMER2_OVF_vect){
  //seconds++;
  seconds += 1; //Adjust to your timer2 time!
}

void setup(void)
{
//  pinMode(10, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(2);
  Sleep.setupTimer2(); //init timer2 to 1 sec
}

void loop(void)
{
  Serial.println("Awake ...");
  Serial.print("Runtime:");
  Serial.println(seconds);
  Serial.print("CPU-Time:");
  Serial.println(millis());
  Serial.println("Entering Sleep mode");
  
  delay(2);
  Sleep.sleep(TIMER2_ON);     // sleep function called here
}

