#include <BayEOS.h>
#include <BaySerial.h>
#include <BayBluetooth.h>



BayBluetooth client=BayBluetooth();

void blink(uint8_t times){
	for(uint8_t i=0;i<times;i++){
	  digitalWrite(13,HIGH);
	  delay(200);
	  digitalWrite(13,LOW);
	  delay(200);
	}
}

void setup(void){
   client.begin(19200,"BayBluetooth");
   pinMode(13,OUTPUT);
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
  blink(client.getStatus());
}
