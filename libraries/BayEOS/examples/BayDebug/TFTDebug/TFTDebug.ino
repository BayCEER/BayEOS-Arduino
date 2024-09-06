
#include <BayDebugTFT.h>


// pin definition for the Uno
#define cs 10
#define dc 9
#define rst 8

TFT TFTscreen(cs, dc, rst);

#define tftcols 20
#define tftrows 10


#define UTFT_AUTOOFF 30

char tftbuffer[tftrows * (tftcols + 1)];

BayTFTDebug client = BayTFTDebug(&TFTscreen, tftbuffer, tftrows, tftcols);

void setup(void) {
  client.begin();
}

void loop(void) {
  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis() / 1000);
  client.addChannelValue(millis() / 100);
  client.addChannelValue(millis() / 10);
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
