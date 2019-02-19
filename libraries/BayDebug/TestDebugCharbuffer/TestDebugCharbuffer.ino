#include <BayDebug.h>
#define BUFFERSIZE 200
char buffer[BUFFERSIZE];

BayDebugCharbuffer client(buffer,BUFFERSIZE);

void setup(void){
  Serial.begin(9600);
}

void loop(void){
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis() / 1000);
  client.sendPayload();
  Serial.println(client.get()); 
  delay(1000);
}

