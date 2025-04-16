#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#define BAUD_RATE 57600

uint8_t buffer[256];
unsigned long max_length, pos;
void setup(void) {
  Serial.begin(BAUD_RATE);
  myBuffer.init(flash); //This will restore old pointers
  max_length = flash.getCapacity();
  Serial.println("ready");
}

void loop(void){
  if(Serial.available()){
    char c=Serial.read();
    switch(c){
      case 'b':
        pos=myBuffer.endPos();
        while (pos!=myBuffer.writePos()){
          uint8_t bytes_read= myBuffer.readBinary(pos,myBuffer.writePos(),255,buffer);
          Serial.write(buffer,bytes_read);
          pos+=bytes_read;
          if(pos>=myBuffer.length()) pos-=myBuffer.length();
      
        }
     break;      
     case 'f':
        pos=0;
        while(pos<max_length){
          flash.readByteArray(pos, buffer, sizeof(buffer));
          Serial.write(buffer,sizeof(buffer));
          pos+=sizeof(buffer);
        }
     break;
      case 's':
      pos=0;
      flash.readByteArray(pos, buffer, sizeof(buffer));
      Serial.write(buffer,sizeof(buffer));
      break;
    }
    
  }
  
}
