#include <BayDebug.h>
#include <RF24.h>

#define RF24CHANNEL 0x47
BayDebug client(Serial);
RF24 radio(9,10);
const uint8_t pipes[6][5] = { 
    {0x12, 0xae, 0x31, 0xc4, 0x45},
    {0x24, 0xae, 0x31, 0xc4, 0x45},
    {0x48},{0x96},{0xab},{0xbf}
};

void setup(void) {
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH); //Disable SPI-Flash
  radio.begin();
  radio.setChannel(RF24CHANNEL);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  for (uint8_t i = 0; i < 6; i++) {
    radio.openReadingPipe(i, pipes[i]);
  }
  client.begin(9600, 1);

  Serial.println("Starting");
  client.sendMessage("RF24-Router started");
  radio.startListening();

}

uint8_t nrf_payload[32];
String origin="RF-x";
void loop(void) {
  uint8_t pipe_num, len;
  while( radio.available(&pipe_num) ) {
 //   client.startRoutedFrame(pipe_num, 0);
    origin[3]='0'+pipe_num;
    client.startOriginFrame(origin,1);
    len = radio.getDynamicPayloadSize();
    radio.read(nrf_payload, len);
    Serial.print("GOT: ");
    for (uint8_t i = 0; i < len; i++) {
      Serial.print(nrf_payload[i], HEX);
      client.addToPayload(nrf_payload[i]);
    }
    Serial.println();
    client.sendPayload();
  }
  delay(1);
}
