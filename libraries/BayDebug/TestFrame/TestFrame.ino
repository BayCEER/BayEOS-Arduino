#include <BayEOS.h>
#include <BayDebug.h>



BayDebug client=BayDebug();


void setup(void){
   client.begin(19200);
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
   
}