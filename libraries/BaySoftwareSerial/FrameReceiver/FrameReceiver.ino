#include <BaySoftwareSerial.h>


BaySoftwareSerial rx_client=BaySoftwareSerial(8,9);

void setup(void){
   rx_client.begin(9600);
   Serial.begin(9600);
   Serial.println("Receiver started...");
}

void loop(void){
  if(! rx_client.readIntoPayload()){
  //got something...
  
    switch(rx_client.getPayload(0)){
    	case BayEOS_DataFrame:
    		Serial.println("Got a data frame");
    		break;
    	case BayEOS_Message:
    		Serial.print("Got a message: ");
                for(uint8_t i=1;i<rx_client.getPacketLength();i++){
                  Serial.print((char)rx_client.getPayload(i));
                }
                Serial.println();
                break;
    	case BayEOS_ErrorMessage:
    		Serial.print("Got a error message: ");
                for(uint8_t i=1;i<rx_client.getPacketLength();i++){
                  Serial.print((char)rx_client.getPayload(i));
                }
                Serial.println();
    		break;
    	default:
    		Serial.println("Got something else...");
    }
  }
}
