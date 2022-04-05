#define SAMPLING_INT 32
#define BOARD_NAME "HX711ESP"
#define SEND_COUNT 60 /*collect 60 measurements before send... */
#define MIN_VOLTAGE 3.8

#include <HX711Array.h>

uint8_t dout[] = {A3, A2, A1, A0};
uint8_t sck = A4;

HX711Array scale;

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

unsigned long last_sent;
#include <BaySerial.h>
BaySerialESP client(Serial, 7);


#include <DS18B20.h>
DS18B20 ds1(2, 0, 1);
DS18B20 ds2(3, 0, 1);
DS18B20 ds3(4, 0, 1);
DS18B20 ds4(6, 0, 1);
float temp;

#include <LowCurrentBoard.h>


uint16_t measurements = SEND_COUNT;

void setup()
{
  ds1.setAllAddr(); //Search for new or removed sensors
  ds2.setAllAddr();
  ds3.setAllAddr();
  ds4.setAllAddr();
  ds1.setResolution(12);
  ds2.setResolution(12);
  ds3.setResolution(12);
  ds4.setResolution(12);

  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 20); // use skip!
  initLCB(); //init time2
  scale.begin(dout, 4, sck); //start HX711Array with 4 ADCs
  scale.set_gain(128);
  scale.power_down();
  startLCB();
  client.begin(38400);
  client.powerUp();
  while (client.isReady()) {
    blinkLED(2);
    delay(2000);
  }
  uint8_t res;
  while (res = client.setName(BOARD_NAME)) {
    if (res == 10 + strlen(BOARD_NAME)) break;
    blinkLED(res);
    delay(res * 500 + 2000);
  }

  blinkLED(client.sendMessage("Board started") + 1);
  client.powerDown();
  delay(2000);
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    ds1.t_conversion(); //Start T-conversion
    ds2.t_conversion(); //Start T-conversion
    ds3.t_conversion(); //Start T-conversion
    ds4.t_conversion(); //Start T-conversion
    delayLCB(700);

    scale.power_up();
    long adc = scale.read_average();
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);
    delay(2);
    analogReference(DEFAULT);
    float bat_voltage = 3.3 * 200 / 100 / 1023 * analogRead(A7);

    scale.power_down();
    //Ausgabe der Spannung in mV bei 10V Anregung!
    float t_mean = 0;
    ds1.readChannel(1, &temp);
    t_mean += temp;
    ds2.readChannel(1, &temp);
    t_mean += temp;
    ds3.readChannel(1, &temp);
    t_mean += temp;
    ds4.readChannel(1, &temp);
    t_mean += temp;
    t_mean /= 4;

    client.startDataFrame();
    client.addChannelValue(millis());
    client.addChannelValue(bat_voltage);
    client.addChannelValue(t_mean);
    client.addChannelValue(adc);

    client.writeToBuffer();
    measurements++;
    if (measurements >= SEND_COUNT & bat_voltage > MIN_VOLTAGE) {
      client.powerUp();
      uint8_t tx_res = client.sendMultiFromBuffer(1000);
      blinkLED(tx_res + 1);
      while (! tx_res && myBuffer.available() && ! ISSET_ACTION(0)) {
        tx_res = client.sendMultiFromBuffer(1000);
        blinkLED(tx_res + 1);
      }
      if (! myBuffer.available()) measurements = 0;
    }
    client.powerDown();
    digitalWrite(POWER_PIN, LOW);
    //Read battery voltage _after_ long uptime!!!

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}
