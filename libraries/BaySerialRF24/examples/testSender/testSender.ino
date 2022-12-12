#include <BaySerialRF24.h>

RF24 radio(9,10);
BaySerialRF24 client(radio,300);

const uint8_t channel=0x62;
const uint8_t adr[] = {0x12, 0xae, 0x31, 0xc4, 0x45};

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  client.init(channel,adr);
}

void loop(){
  Serial.print("Sending ... ");
  unsigned long t=millis();
  uint8_t res;
//  res=client.sendMessage("This is a long message. For more than 32 bytes RF24 has to send several packages  .... ");
//  res=client.sendMessage("one frame");

  client.startDataFrameWithOrigin(0x41,"Test-Board",1,1);
  for(uint8_t i=0;i<10;i++){
    client.addChannelValue(i);
  }
  client.addChecksum();
  res=client.sendPayload();
  t=millis()-t;
  
  Serial.print("\t");
  Serial.print(t);
  Serial.print("ms ");
  if(res){
    Serial.print("failed res=");
    Serial.println(res);
  } else
    Serial.println("ok");
  //delay(5000);
  
}
  
