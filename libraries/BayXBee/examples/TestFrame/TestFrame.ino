#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>


BayXBee client=BayXBee(); 


void setup(void){
  client.begin(38400);
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame();
   client.addChannelValue(millis()/1000);     
   client.addChannelValue(analogRead(A0));     
   client.sendPayload();
   
// client.sendMessage("Just a message ;-)");
                                                                                                                                              
// client.sendError("Just a test error message ;-)");                                                                                           
                                                                                                                                              
  delay(5000);                                                                                                                                
                                                                                                                                              
}
