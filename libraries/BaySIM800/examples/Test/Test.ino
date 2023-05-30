#include <BayEOSBufferSPIFlash.h>
#include <BaySIM800.h>

BaySIM800 client = BaySIM800(Serial);
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

void blink(uint8_t times) {
  pinMode(LED_BUILTIN, OUTPUT);
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    times--;
    if (! times) return;
    delay(500);
  }
}

void setup(void) {
  Serial.begin(38400);
  Serial.println("starting");
  delay(100);
  //  client.softSwitch();
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  client.readConfigFromStringPGM(
    PSTR("http://bayeos.bayceer.uni-bayreuth.de/gateway/frame/saveFlat|import@IT|import|SIM800_HTTPS|iot.1nce.net||||"));
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  client.setBuffer(myBuffer);
  blink(client.begin(38400) + 1);
}

void loop(void) {
  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis() / 1000);
  client.writeToBuffer();
  blink(client.sendMultiFromBuffer() + 1);

  delay(5000);

}
