#include <BayEOS.h>
#include <SoftwareSerial.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>


BayGPRSsoftserial client = BayGPRSsoftserial(3, 4);

void setup(void) {
  client.readConfigFromStringPGM(
    PSTR("GATEWAY_IP|80|path|user|password|sender|apn|||pin"));
  client.begin(38400);

}

void loop(void) {
  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis() / 1000);
  client.sendPayload();
  delay(5000);

}

