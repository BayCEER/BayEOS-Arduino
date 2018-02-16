
#include <BayDebugTFT.h>

/*
UTFT myGLCD(ITDB18SP,33,32,31,35,34);
//UTFT myGLCD(ITDB18SP,35,34,33,37,36);   //IBoard pro 
#define utftcols 16
#define utftrows 13
*/
UTFT myGLCD(ITDB24E_8,38,39,40,41); //240x320 Parallel
#define utftcols 30
#define utftrows 26

//36+37 for VCC + GND

#define UTFT_AUTOOFF 30

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
