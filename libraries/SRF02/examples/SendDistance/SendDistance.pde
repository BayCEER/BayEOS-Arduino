#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>
#include <bayxbee_const.h>
#include <inttypes.h>
#include <SRF02.h>


// Initialize with default settings 
SRF02 srf = SRF02();
BayXBee xbee=BayXBee();

void setup()
{
 // open the serial port:
 xbee.begin(9600); 
}

void loop()
{
  xbee.startDataFrame(BayOES_Int16le); 
  xbee.addToPayload((uint8_t) 0); //Offset 0 
  xbee.addToPayload((uint16_t)srf.getDistance()); 
  xbee.sendPayload();
  delay(10*1000); 
}

