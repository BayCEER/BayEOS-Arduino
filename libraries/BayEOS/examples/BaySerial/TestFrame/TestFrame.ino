#include <BaySerial.h>

BaySerial client = BaySerial(Serial);

void setup(void)
{
   client.begin(9600);
}

long package_count = 1;
void loop(void)
{
   client.startDataFrame();
   client.addChannelValue(millis() / 1000);
   client.addChannelValue(package_count);
   client.sendPayload();
   package_count++;

   delay(5000);
}
