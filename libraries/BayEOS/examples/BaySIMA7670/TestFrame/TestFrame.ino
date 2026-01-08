#include <BayEOSBufferSPIFlash.h>
#include <BaySIMA7670.h>


// You have to change the BAUD-rate of the modul manualy with USB
// AT+IPREX=38400

// Config string.
// Gateway-url|login|password|Origin (== Board unique identifier)|apn of sim-card|apn-user|apn-pw|PIN
#define CONFIG "https://bayeos.bayceer.uni-bayreuth.de/gateway/frame/saveFlat|import@IT|import|A7670E|iot.1nce.net||||"

BaySIMA7670 client(Serial, 6);
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <LowCurrentBoard.h>



void setup(void)
{
  initLCB();                                    // init time2
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  blinkLED(3);
  delay(5000);
  client.readConfigFromStringPGM(PSTR(CONFIG));
  myBuffer.init(flash); // This will restore old pointers
  // myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); // This will move read pointer to write pointer
  client.setBuffer(myBuffer);
  adjust_OSCCAL();
  blinkLED(client.begin(38400) + 1);
}

long package_count = 1;
void loop(void)
{
  client.now();
  // Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis() / 1000);
  client.addChannelValue(package_count);
  client.writeToBuffer();
  package_count++;

  if ((package_count % 5) == 1)
    blinkLED(client.sendMultiFromBuffer() + 1);

  delay(5000);
}
