#include <BayEOSBufferSPIFlash.h>
#include <BayTCPSim900.h>

BayGPRS client = BayGPRS(Serial);
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
  //Serial.begin(9600);
  //  client.softSwitch();
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  digitalWrite(A3, HIGH);
  client.readConfigFromStringPGM(
    PSTR("132.180.112.128|80|gateway/frame/saveFlat|import@IT|import|GPRS-ACK1|iot.1nce.net||||"));
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  client.setBuffer(myBuffer);
  blink(client.begin(38400) + 1);
  digitalWrite(A3, LOW);

}

void loop(void) {
  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis() / 1000);
  client.writeToBuffer();
  blink(client.sendMultiFromBufferWithAckPayload() + 1);

  if (client.getPacketLength()) {
    delay(2000);
    blink(client.getPacketLength());
    //Got a AckPayload
    if (client.getPayload(0) == BayEOS_Action) {
      uint8_t pin = client.getPayload(1);
      uint8_t value = client.getPayload(2);
      if (pin < 5 && pin > 1) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, value);
        client.createActionResponse(pin, BayEOS_ActionSuccess);
      } else {
        client.createActionResponse(pin, BayEOS_ActionFailed);
        client.addToPayload("No valid pin"); //additional error information
      }
      client.writeToBuffer(); //will be send with next send action. 
    }
  }
  delay(5000);

}
