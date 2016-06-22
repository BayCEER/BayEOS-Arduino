#include <BayEOS.h>
#include <BayDebug.h>



BayDebug client=BayDebug(Serial);


void setup(void){
   client.begin(9600);
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   client.sendPayload();

   client.startDataFrame(BayEOS_ChannelFloat32le);
   client.addChannelValue(millis()/1000,12);     
   client.sendPayload();
   
   client.startDataFrame(BayEOS_LabelledChannelFloat32le);
   client.addChannelValue(millis()/1000,"cpu");     
   client.addChannelValue(analogRead(A0),"A0");     
   client.sendPayload();
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
   
}
