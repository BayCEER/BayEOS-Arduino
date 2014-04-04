#include <UTFT.h>
#include <TFTStream.h>

UTFT myGLCD(ITDB18SP,33,32,31,35,34);

#define utftcols 16
#define utftrows 13
char utftbuffer[utftrows*(utftcols+1)];
TFTStreamDev TFT(&myGLCD,utftbuffer,utftrows,utftcols);

void setup(void){
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  digitalWrite(37,HIGH);
  digitalWrite(36,LOW);
  TFT.begin(); 
}

void loop(void){
   TFT.print("Start: ");
   TFT.print(millis());
   TFT.println();
 //  TFT.println("-----------");
   TFT.flush();
   delay(1000);
}
