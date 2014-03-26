#include <Wire.h>
#include <SRF02.h>

// Initialize with default settings 
SRF02 srf = SRF02(0x70, SRF02_MICROSECONDS);
void setup()
{
 // open the serial port:
 Serial.begin(9600);
}

void loop()
{
 Serial.print("Mikroseconds: ");
 Serial.println(srf.getDistance());
 delay(1000);
}

