/*
  Configure Stalker Sketch
  - Reset Buffer
  - Set Clock
*/

#include <Wire.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>


DS3231 myRTC; //Seduino 2.2

BayEOSBufferSDFat myBuffer;
char buffer[49];

#define CONFIG_XBEE 0
#define XBEE_SM 1
#define XBEE_MYID 2
#define XBEE_PANID 2712
#define XBEE_CH 12


void setup() {
  Serial.begin(9600);

#if CONFIG_XBEE
  delay(100);
  Serial.print("+++");
  waitForOK();
  Serial.print("ATMY");
  Serial.print(XBEE_MYID);
  Serial.print("\r\n");
  waitForOK();
  Serial.print("ATAP2\r\n");
  waitForOK();
  Serial.print("ATSM");
  Serial.print(XBEE_SM);
  Serial.print("\r\n");
  waitForOK();
  Serial.print("ATBD5\r\n");
  waitForOK();
  Serial.print("ATID");
  Serial.print(XBEE_PANID);
  Serial.print("\r\n");
  waitForOK();
  Serial.print("ATCH");
  Serial.print(XBEE_CH);
  Serial.print("\r\n");
  waitForOK();
  Serial.print("ATWR\r\n");
  if (! waitForOK()) {
    pinMode(13, OUTPUT);
    for (uint8_t i = 0; i < 5; i++) {
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
      delay(500);
    }
  }
#endif

  delay(1000);
  Serial.println("Starting");

  //Set 4 for EthernetShield, 10 for Stalker
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    delay(10000);
    Serial.println("No SD!");
  }

  myBuffer = BayEOSBufferSDFat(200000000, 1);
  myBuffer.setRTC(myRTC, false); //Nutze RTC jedoch relativ!
  Serial.print("Buffer Ok - Size:");
  Serial.println(myBuffer.readPos());
  Serial.println("press 'r' to delete Buffer");
  if (waitForChar('r')) {
    Serial.println("deleting Buffer");
    myBuffer.reset();
    delay(1000);
  }

  Wire.begin();
  myRTC.begin();
}

void loop() {
  DateTime now = myRTC.now();
  Serial.print("UTC-time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  Serial.println("press s to set");

  if (Serial.available() && Serial.read() == 's') {
    do {
      delay(3);
      Serial.read();
    } while (Serial.available());

    Serial.println("Enter new time in Format YY/MM/DD HH:MM:SS+TZ");
    do {
      delay(1);
    } while (Serial.available() < 20);

    uint8_t i = 0;
    while (Serial.available()) {
      buffer[i] = Serial.read();
      i++;
    }
    uint16_t y = atoi(buffer + 0) + 2000;
    uint8_t m = atoi(buffer + 3);
    uint8_t d = atoi(buffer + 6);
    uint8_t hh = atoi(buffer + 9);
    uint8_t mm = atoi(buffer + 12);
    uint8_t ss = atoi(buffer + 15);
    DateTime dt = DateTime (y, m, d, hh, mm, ss);
    dt = DateTime(dt.get() - (3600L * atoi(buffer + 17))); //Adjust for Timezone!
    myRTC.adjust(dt);
  }

  do {
    delay(3);
    Serial.read();
  } while (Serial.available());
  delay(1000);

}


uint8_t waitForChar(char c) {
  int timeout = 6000;
  do {
    if (Serial.available() && Serial.read() == c) {
      Serial.println();
      return 1;
    }
    if (timeout % 1000 == 0) {
      Serial.print((timeout / 1000));
      Serial.print("..");
    }
    timeout--;
    delay(1);
  } while (timeout > 0);
  Serial.println();
  return 0;

}

uint8_t waitForOK(void) {
  uint8_t timeout = 254;
  while (timeout) {
    delay(5);
    if (Serial.available() > 1) {
      if (Serial.read() == 'O') timeout = 255;

      while (Serial.available()) {
        delay(5);
        Serial.read();
      }
      if (timeout == 255) return 0;
    }
    timeout--;
  }

  return 1;
}

