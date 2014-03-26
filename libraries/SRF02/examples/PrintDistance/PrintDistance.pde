#include <Wire.h>
#include <SRF02.h>

// Initialize with default settings 
SRF02 srf = SRF02();
void setup()
{
 // open the serial port:
 Serial.begin(9600);
}

void loop()
{
 Serial.print("Distance: ");
 Serial.print(srf.getDistance());
 Serial.println(" cm");
 delay(1000);
}

