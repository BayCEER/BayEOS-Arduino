#include <BayEOS.h>
#include <BaySerial.h>



BaySerial client=BaySerial(Serial);


void setup(void){
   client.begin(9600);
}

void loop(void){
  /*
   * Using LabeledChannelType increses the frame size by
   * 1+strlen(Label)+sizeof(datatype) per added channel
   * 
   * Please keep in mind, that the payload of the BayEOS object
   * is limited to 100 byte
   */
   client.startDataFrame(BayEOS_LabeledChannelFloat32le);
   client.addChannelValue(millis()/1000,"cpu");     
   client.addChannelValue(analogRead(A0),"A0");     
   client.sendPayload();
  
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
}
