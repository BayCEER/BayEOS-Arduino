#include <BayEOS.h>
#include <BayTCPSim900.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <Sleep.h>
#include <RAD7Serial.h>

/*
 * Configuration
 */
#define RX_LED A6
#define TX_LED A7
#define POWER_AD_PIN A4
#define DIVIDER (470.0+100.0)/100.0
#define SD_CSPIN 4
#define GPRS_PIN 46 /*GBoard */
#define GPRS_RESET 47
#define TX_SERIAL Serial2
#define RAD7_SERIAL Serial1
#define SAMPLING_INT 900000L
#define TRY_INT 180000L
#define SENDING_INT 10000L

RTC_SIM900 myRTC;
BayGPRS client = BayGPRS(TX_SERIAL, GPRS_PIN, GPRS_RESET);
BayEOSBufferSDFat myBuffer;
RAD7Serial myRAD7(RAD7_SERIAL, client);

unsigned long last_data_try, last_data, last_buffered_data;
int res;

volatile uint16_t wdcount = 0;
volatile uint8_t wdreset = 0;
volatile uint8_t tx_blink = 0;
volatile uint8_t rx_blink = 0;

ISR(WDT_vect) {
  wdcount++;
  if (wdreset) {
    wdcount = 0;
    wdreset = 0;
  }
  if (wdcount > 4800) { //no action for more than 1200 sec.
    asm volatile (" jmp 0"); //restart programm
  }
#ifdef TX_LED
  if (tx_blink) {
    if(digitalRead(TX_LED)) tx_blink--;
    digitalWrite(TX_LED,!digitalRead(TX_LED)); 
  }
#endif
#ifdef RX_LED
  if (rx_blink) {
    if(digitalRead(RX_LED)) rx_blink--;
    digitalWrite(RX_LED,!digitalRead(RX_LED)); 
  }
#endif
}

void setup(void) {
  Serial.begin(9600);
  RAD7_SERIAL.begin(1200);
  Sleep.setupWatchdog(WDTO_250MS); //250ms
  Serial.println("Starting...");
  delay(10);
#ifdef RX_LED
  pinMode(RX_LED, OUTPUT);
#endif
#ifdef TX_LED
  pinMode(TX_LED, OUTPUT);
#endif
  rx_blink = 3;
  delay(500);
  tx_blink = 2;
  delay(1500);

  Serial.println("SD");

  while (!SD.begin(SD_CSPIN)) {
    rx_blink = 2;
    tx_blink = 2;
    delay(2000);
  }

  delay(1000);
  rx_blink = 1;
  delay(1000);
  Serial.println("GPRS");
  client.readConfigFromFile("GPRS.TXT");
  res = client.begin(38400);
  tx_blink = res + 1;
  delay(500 * res + 1000);
  wdreset = 1;
  myBuffer = BayEOSBufferSDFat(2000000000L, 1); //Append mode!
  myRTC.adjust(client.now());
  myBuffer.setRTC(myRTC, 0); //Relative Mode...
  client.setBuffer(myBuffer, 0);
  last_data -= SAMPLING_INT;
  last_data_try -= SAMPLING_INT;
  delay(1000);
  rx_blink = 1;
  delay(1000);
  client.startFrame(BayEOS_Message);
  client.addToPayload("FW ");
  client.addToPayload(__DATE__);
  res = client.sendPayload();
  tx_blink = res + 1;
  delay(res * 500 + 1000);
  if (res)
    client.writeToBuffer();
 
  Serial.println("Setup OK");
}

void loop(void) {
  wdreset=1;
  if (myBuffer.available() && (millis() - last_buffered_data) > SENDING_INT) {
    Serial.println("sending");
    res = client.sendMultiFromBuffer();
    tx_blink = res + 1;
    last_buffered_data = millis();
  }

  if ((millis() - last_data) > SAMPLING_INT
      && (millis() - last_data_try) > TRY_INT ) {
    Serial.println("reading");
    last_data_try = millis();

    if (myRAD7.get_rnumber() > 100)
      myRAD7.deleteData();

    res = myRAD7.readData();
    if (res > 0) last_data = millis();
  }
}

