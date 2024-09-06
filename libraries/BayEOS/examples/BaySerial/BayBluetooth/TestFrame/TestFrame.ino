#include <BayBluetooth.h>

BayBluetooth client=BayBluetooth(Serial);

void setup(void){
   digitalWrite(7, HIGH); // Pull Up 
   client.begin(38400,"BayBluetooth");
}

void loop(void){
   if (!digitalRead(7)) 
     client.inquirable();
   
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
}
