#include <BayEOS.h>
#include <BayDebug.h>

#include <UTFT.h>
#include <TFTStream.h>
#include <BayDebugTFT.h>

UTFT myGLCD(ITDB18SP,33,32,31,35,34);
//36+37 for VCC + GND
//UTFT myGLCD(ITDB18SP,35,34,33,37,36);   //IBoard pro 

#define UTFT_AUTOOFF 30
#define utftcols 16
#define utftrows 13

char utftbuffer[utftrows*(utftcols+1)];

BayTFTDebug client=BayTFTDebug(&myGLCD,utftbuffer,utftrows,utftcols);

void setup(void){
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  digitalWrite(37,HIGH);
  digitalWrite(36,LOW);
  client.begin();
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   client.addChannelValue(millis()/100);     
   client.addChannelValue(millis()/10);     
   client.sendPayload();
   client.flush();
   client.println("Just inserted");
   client.flush();
 //  client.sendMessage("Just a message ;-)");
  // client.flush();

  // client.sendError("Just a test error message ;-)");
  // client.flush();
  delay(5000);
   
}
