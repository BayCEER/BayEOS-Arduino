/****************************************************************
 * 
 * Sketch for MLX90614 connected to low power Arduino
 *
 ***************************************************************/ 
#include <BayEOSBuffer.h>
#include <Wire.h>
#include <RTClib.h>
#include <Sleep.h>
#include <MLX90614.h>

volatile uint16_t ticks; //16 ticks per second

MLX90614 mlx = MLX90614();

RTC_Timer2 myRTC;
ISR(TIMER2_OVF_vect){
  ticks++;
  if((ticks % 16)==0){
    myRTC._seconds += 1; 
  }
}

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  Wire.begin();
  mlx.enterSleepMode();
  Serial.begin(9600);
  Serial.println("Starting");
  ticks=63;

}

void loop()
{
  if((ticks%64)==58){
    //wake up mlx with no additional delay
    //we need 100ms uptime to get a resonable measurement
    //We pass the time while sleeping!
    mlx.exitSleepMode(0);
  }
  
  if((ticks%64)==0){
    Serial.print(mlx.readAmbientTempC());
    Serial.print("\t");
    Serial.print(mlx.readObjectTempC());
    Serial.print("\n");
    mlx.enterSleepMode();

    delay(50);
  }
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}

