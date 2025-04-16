/*******************************************************
 Rain gauge sketch with esp01 or SIM800

*******************************************************/

#define BOARD_NAME "RegenWaage"
#define GATEWAY_IP "192.168.2.108"
#define GATEWAY_USER "import"
#define GATEWAY_PW "import"

// SIM800-Config string.
// Gateway-url|login|password|Origin (== Board unique identifier)|apn of sim-card|apn-user|apn-pw|PIN
#define EEPROM_BUFFER_STATUS_POS 300
#define EEPROM_BUFFER_STATUS_BYTE 0xc0 /* indicates whether there is valid data in the flash buffer */
// Change the EEPROM_BUFFER_STATUS_BYTE if you want the board to reset the buffer once

// #define SIM800_CONFIG "http://132.180.112.128/gateway/frame/saveFlat|gfp@gfp|gfp|RegenWaage|iot.1nce.net||||"

#define BAT_DIVIDER (100.0 + 100.0) / 100.0
#define SAMPLING_INT 30
#define MITTELUNG 15
#define TMEAN1 20       /* 10 min */
#define TMEAN2 30       /* 5h */
#define SEND_COUNT 60   /*collect 60 measurements before send... */
#define MIN_VOLTAGE 3.7 /*minimum voltage for send operation */
#define MAX_EVAP 1.5    /*evaporation in mm/h at 20°C*/
// exp(17,62 * t / ( 243,12 + t ))
#define MIN_SLOPE 0.6     /*minimum slope in mm/h */
#define MAX_DEVIATION 2.0 /*mm maximum difference between raw and current */

#define INIT_CAL 0

#if INIT_CAL
// Calibration Data for Regenwaage
float slope = -0.0002289260;
float m[] = {-19776.63, -1269.86, 7228.99, -1572.55};

#endif

#include <BayEOSBufferSPIFlash.h>
unsigned long last_sent;
SPIFlash flash(8);             // Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer; // BayEOS Buffer

#ifdef SIM800_CONFIG
#include <BaySIM800.h>
BaySIM800 client = BaySIM800(Serial);
#else
#include <BaySerial.h>
BaySerialESP client(Serial, 7);
#endif

#include <HX711Array.h>
#include <NTC.h>
uint8_t dout[] = {6};
uint8_t sck = 3;

HX711Array scale;
NTC_HX711 ntc(scale, 2 * 470000, 3.0); // adjust resistor values
ScaleTCal cal0;

// Some settings and functions for LowCurrentBoard
#define RESET_COUNT 5

#include <LowCurrentBoard.h>
#include <math.h>

#include <RainGauge.h>
uint16_t measurements = SEND_COUNT;

void setup()
{
  initLCB();
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash);                         // This will restore old pointers
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS); // use the rtc clock but relative
  client.setBuffer(myBuffer);                   // connect the buffer to the transport client

  digitalWrite(POWER_PIN, HIGH); // power up GPRS-Modem
  startLCB();                    // some settings and blink three times
  delayLCB(1000);
  adjust_OSCCAL(); // tune clock of ATMega to make serial communication more stable
  delayLCB(1000);
#ifdef SIM800_CONFIG
  client.readConfigFromStringPGM(PSTR(SIM800_CONFIG)); // read GPRS config into RAM
  blinkLED(client.begin(38400) + 1);                   // start the GPRS-Modem (e.g. connect to network)
  delay(2000);
#else
  client.begin(38400);
  client.powerUp();
  while (client.isReady())
  {
    blinkLED(2);
    delay(2000);
  }
  uint8_t res;
  while (res = client.setConfig(BOARD_NAME, BaySerialESP_NAME))
  {
    if (res == 10 + strlen(BOARD_NAME))
      break;
    blinkLED(res);
    delay(res * 500 + 2000);
  }
  while (res = client.setConfig(GATEWAY_IP, BaySerialESP_GATEWAY))
  {
    if (res == 10 + strlen(GATEWAY_IP))
      break;
    blinkLED(res);
    delay(res * 500 + 2000);
  }
  while (res = client.setConfig(GATEWAY_USER, BaySerialESP_USER))
  {
    if (res == 10 + strlen(GATEWAY_USER))
      break;
    blinkLED(res);
    delay(res * 500 + 2000);
  }
  while (res = client.setConfig(GATEWAY_PW, BaySerialESP_PW))
  {
    if (res == 10 + strlen(GATEWAY_PW))
      break;
    blinkLED(res);
    delay(res * 500 + 2000);
  }
#endif
  blinkLED(client.sendMessage("Board started") + 1);
#ifdef SIM800_CONFIG
  while (true)
  {
    unsigned long time = client.now().get();
    if (time > 3600L * 24 * 365 * 20)
    {
      myRTC.adjust(time);
      break;
    }
    blinkLED(2);
    delay(2000);
  }
  if (EEPROM.read(EEPROM_BUFFER_STATUS_POS) != EEPROM_BUFFER_STATUS_BYTE)
  {
    myBuffer.reset();
    EEPROM.write(EEPROM_BUFFER_STATUS_POS, EEPROM_BUFFER_STATUS_BYTE);
  }
  Serial.end(); // Stop Serial (avoids power leakage via TX Pin)

#else
  myBuffer.skip();
  client.powerDown();
#endif

  delay(2000);

  scale.begin(dout, 1, sck); // start HX711Array with 1 ADCs
#if INIT_CAL
  cal0.saveConf(slope, m);
#endif
  cal0.readConf();
}

void loop()
{
  if (ISSET_ACTION(0))
  {                         // will be set by timer2 interrupt each SAMPING_INT seconds
    handleAction0();        // will fill a data.frame
    client.writeToBuffer(); // Save data to flash
    measurements++;
    adjust_OSCCAL();
    // increase measurement count
    if (measurements >= SEND_COUNT & bat_voltage > MIN_VOLTAGE)
    { // time to send data
#ifdef SIM800_CONFIG
      digitalWrite(POWER_PIN, HIGH); // power up modem
#else
      client.powerUp();
#endif
      delayLCB(1000);

      uint8_t tx_res = client.sendMultiFromBuffer(1000);
      blinkLED(tx_res + 1);
      while (!tx_res && myBuffer.available())
      {
        tx_res = client.sendMultiFromBuffer(1000); // send 3000 bytes from flash storage
        blinkLED(tx_res + 1);
#ifdef SIM800_CONFIG
        unsigned long time = client.now().get();
        if ((myRTC.get() - time) < 1000 || (time - myRTC.get()) < 1000 || (!myBuffer.available() && time > 20 * 365 * 24 * 3600))
          myRTC.adjust(time);

#endif
        if (ISSET_ACTION(0))
          break;
      }
      if (!myBuffer.available())
        measurements = 0; // all data sent from flash storage
#ifdef SIM800_CONFIG
      Serial.end();
#else
      client.powerDown();
#endif
      digitalWrite(POWER_PIN, LOW);
    }
  }
  sleepLCB(); // board stops execution here until timer2 wakes up the board again (typ. 16 per second)
  // Most time the boards just wakes up, runs the timer2-ISR and as there is nothing to do just
  // runs directly into sleepLCB() again.
}
