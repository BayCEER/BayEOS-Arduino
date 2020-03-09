//Connector Sketch to allow to connect to several loggers via RF24
//Switching is done by INT0. Feedback via NOKIA5110-Display

struct RadioTarget
{
  uint8_t channel;
  uint64_t addr;
  char name[15];
};

//define your logger connections here.
//make sure that the name does not exceed 14 characters (ASCII)!!!
const RadioTarget targets[] PROGMEM = {
  {0x67, 0x45c431ae12, "ADC-R-Test"},
  {0x70, 0x45c431ae12, "MPLEX8-Test"},
  {0x70, 0x45c4e3f046, "BME280 - GFP"},
  {0x72, 0x3bc43e0f46, "BME280 - TEST"},
};

uint8_t num_radios = sizeof(targets)/sizeof(RadioTarget);


#define LCD_LED 7

#include <BaySerial.h>
#include <BayRF24.h>
BayRF24 client(9, 10);
RF24 radio(9, 10);


#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// pin A0 - Serial clock out (SCLK)
// pin A1 - Serial data out (DIN)
// pin A2 - Data/Command select (D/C)
// pin A3 - LCD chip select (CS)
// pin A4 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(A0, A1, A2, A3, A4);

#include <LowCurrentBoard.h>

unsigned long _connected, _last_blink, _wait_start;
uint8_t _connected_flag;

volatile uint8_t int0_flag;
void int0_isr(void) {
  int0_flag = 1;
}

uint8_t current_radio = 0;
RadioTarget target;

void init_radio() {
  memcpy_P(&target,targets+current_radio,sizeof(RadioTarget));  
  radio.begin();
  radio.setChannel(target.channel);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  radio.openWritingPipe(target.addr);
  radio.openReadingPipe(0, target.addr);
  radio.startListening();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.print("#");
  display.print(current_radio + 1);
  display.print("/");
  display.print(num_radios);
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print(target.name);
  display.setCursor(0, 24);
  display.print("C:0x");
  display.print(target.channel, HEX);
  display.setCursor(0, 32);
  display.print("P:0x");
  uint8_t i=4;
  uint8_t* p=(uint8_t*)(& target.addr);
  while(i<5){
    display.print(*(p+i),HEX);
    i--;
  }
  display.setCursor(0, 40);
  display.print("WAITING");
  display.display();
  _connected = myRTC.get() - 60;
  _wait_start = millis();
  _connected_flag = 0;

}

void setup() {
  display.begin();
  pinMode(LCD_LED, OUTPUT);
  initLCB(); //init time2
  init_radio();
  adjust_OSCCAL();
  Serial.begin(38400);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), int0_isr, RISING);
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
  //  radio.openWritingPipe (adr[current_radio]);
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
  if (int0_flag) {
    current_radio++;
    if (current_radio >= num_radios) current_radio = 0;
    init_radio();
    delay(50);
    int0_flag = 0;
  }

  if ((myRTC.get() - _connected) < 35) {
    digitalWrite(LCD_LED, LOW);
    if (! _connected_flag) {
      display.fillRect(0, 40, 84, 8, 0);
      display.setCursor(0, 40);
      display.print("CONNECTED :-)");
      display.display();
      _connected_flag = 1;
    }
  } else {
    if (_connected_flag) {
      display.fillRect(0, 40, 84, 8, 0);
      display.setCursor(0, 40);
      display.print("NO CONNECTION");
      display.display();
      _connected_flag = 0;
    }
    if ((millis() - _last_blink) > 1000) {
      _last_blink = millis();
      digitalWrite(LCD_LED, HIGH);
      display.fillRect(48, 40, 84 - 48, 8, 0);
      display.setCursor(48, 40);
      display.print((millis() - _wait_start) / 1000);
      display.display();
      digitalWrite(LCD_LED, LOW);
    }
  }

  if (! radio.available()) delay(3);
  while (radio.available()) {
    while (Serial.available()) Serial.read();
    _connected = myRTC.get();
    len = radio.getDynamicPayloadSize();
    if (! len) {
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
