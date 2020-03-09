//Connector Sketch to allow logger communications via RF24

#include <BaySerial.h>
#include <BayRF24.h>
BayRF24 client(9, 10);
RF24 radio(9, 10);

//Pipes and channel must be the same as in sender
const uint8_t channel = 0x67;
const uint8_t adr[] = {0x12, 0xae, 0x31, 0xc4, 0x45};

#include <LowCurrentBoard.h>

unsigned long _connected, _last_blink;

void setup() {
  initLCB(); //init time2
  adjust_OSCCAL();
  radio.begin();
  radio.setChannel(channel);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  radio.openWritingPipe(adr);
  radio.openReadingPipe(0, adr);
  radio.startListening();
  Serial.begin(38400);
  _connected = myRTC.get() - 60;
}

char buffer[32];
uint8_t len, pipe, res;
uint8_t w_counter = 0;
uint8_t r_counter = 0;

void sendBuffer(uint8_t& res) {
  if (len < 2) return;
  digitalWrite(LED_BUILTIN, LOW);
  radio.stopListening();
  w_counter++;
  if (! w_counter) w_counter++;
  buffer[0] = w_counter;
  while (radio.available()) radio.flush_rx();
  radio.openWritingPipe (adr);
  delay(5);
  res = radio.write(buffer, len);
  uint8_t curr_pa = 0;
  while (!res && curr_pa < 4) {
    radio.setPALevel((rf24_pa_dbm_e) curr_pa);
    delayMicroseconds(random(1000));
    res = radio.write(buffer, len);
    curr_pa++;
  }
  if (! res) radio.setPALevel(RF24_PA_HIGH);
  else _connected = myRTC.get();
  radio.flush_tx();
  radio.startListening();
  len = 1;
}


void loop() {
  if ((myRTC.get() - _connected) < 30)
    digitalWrite(LED_BUILTIN, HIGH);
  else if((millis()-_last_blink)>200){
    _last_blink=millis();
    digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
  }

  if (! radio.available()) delay(3);
  while (radio.available()) {
    while(Serial.available()) Serial.read();
    _connected = myRTC.get();
    len = radio.getDynamicPayloadSize();
    if(! len){
      delay(5);
      continue;
      
    }
    radio.read(buffer, len);
    if (len > 1 && buffer[0] != r_counter) {
      r_counter = buffer[0];
      Serial.write(buffer + 1, len - 1);
    }
    delay(3);
  }

  if (Serial.available()) {
    len = 1;
    res = 0;
    while (Serial.available()) {
      buffer[len] = Serial.read();
      len++;
      if (! Serial.available()) {
        delay(5);
      }
      if (len > 16) {
        sendBuffer(res);
      }
    }
    if (len > 1) {
      sendBuffer(res);
    }
  }
}
