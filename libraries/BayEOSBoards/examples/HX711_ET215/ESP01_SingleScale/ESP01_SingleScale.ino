#define SAMPLING_INT 32
#define BOARD_NAME "HX711ESP_Single"
#define SEND_COUNT 60 /*collect 60 measurements before send... */
#define MIN_VOLTAGE 3.8

#include <HX711Array.h>
#include <NTC.h>

uint8_t dout[] = {6};
uint8_t sck = 3;

HX711Array scale;
Scale4PointCal cal;
NTC_HX711 ntc(scale, 2*470000, 3.0); //adjust resistor values

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

unsigned long last_sent;
#include <BaySerial.h>
BaySerialESP client(Serial, 7);


float temp;

#include <LowCurrentBoard.h>


uint16_t measurements = SEND_COUNT;

void setup()
{
 
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 20); // use skip!
  initLCB(); //init time2
  scale.begin(dout, 1, sck); //start HX711Array with 1 ADCs
  scale.set_gain(128);
  scale.power_down();
  cal.readConf();

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
    ntc.readResistance();
    temp = ntc.getTemp(0);

    scale.power_up();
    scale.read_average(1);
    long adc = scale.read_average();
    pinMode(7, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(2);
    analogReference(DEFAULT);
    float bat_voltage = 3.3 * 200 / 100 / 1023 * analogRead(A7);

    client.startDataFrame();
    client.addChannelValue(millis());
    client.addChannelValue(bat_voltage);
    client.addChannelValue(temp);
    client.addChannelValue(adc);
    client.addChannelValue(cal.getWeight(adc, temp));

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

  }

  sleepLCB();

}
