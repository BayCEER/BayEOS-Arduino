#include <CO2K30.h>

CO2K30 sensor;
#define SAMPLING_INT 600
#define WARMUP 30
#define NUMBER_OF_CHANNELS 2
#define BAT_DIVIDER (3.3 * (220 + 100) / 100 / 1023)
#define BAT_WARNING 5800
#define MAX_TRY 30

#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>
#include <math.h>

#define CONNECTED_PIN 9
#define TICKS_PER_SECOND 4
uint8_t connected = 0;

BaySerial client(Serial);
SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>

//Add your sensor measurements here!
float values[NUMBER_OF_CHANNELS ];
uint16_t count;
unsigned long last_measurement, warmup_start;

uint16_t try_count;
uint8_t warmup;
void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
      values[i] = 0;
    }
  }

  //Add your sensor measurements here!
  float v = sensor.readCO2() * 10;
//  float v=400.0;
  if(isnan(v)) v=-999;
  values[1] += v;

  analogReference(DEFAULT);
  myLogger._bat = (BAT_DIVIDER * analogRead(A7)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;

  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }
}

volatile uint8_t int0;
void int0_isr(void) {
//  int0 = 1;
}

void setup() {
  pinMode(POWER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), int0_isr, FALLING);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, SAMPLING_INT, BAT_WARNING); //min_sampling_int = 60, LOW BAT Warning 3700mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  initLCB(); //init time2
  int0 = 0;
}

void powerUp(void) {
  if(! digitalRead(POWER_PIN)){
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(POWER_PIN, HIGH);
    warmup_start = myRTC.get();
    warmup = 1;
    try_count=0;
  }

}

void powerDown(void) {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(POWER_PIN, LOW);
  warmup = 0;
}

void loop() {
  // Calibration pressed!!
  if (int0) {
    powerUp();
    while((myRTC.get() - warmup_start) < WARMUP){
      delayLCB(200);
    }  
    sensor.begin();
    sensor.zeroCalibration();
    int0 = 0;
  }

  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;

  uint8_t m_flag = (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                    (myRTC.get() - last_measurement) >= SAMPLING_INT));

  if (m_flag && ! digitalRead(POWER_PIN)) {
    powerUp();
    m_flag = 0;
  }
  if (m_flag && (myRTC.get() - warmup_start) < WARMUP) {
    m_flag = 0;
  }

  if (m_flag) {    
    sensor.begin();
    measure();
    last_measurement = myRTC.get();
    warmup = 0;
  }
  myLogger.run();

  if (! connected && myLogger._logging_disabled) {
    digitalWrite(LED_BUILTIN, HIGH);
    delayLCB(200);
    digitalWrite(LED_BUILTIN, LOW);
    delayLCB(800);
  }

  //sleep until timer2 will wake us up...
  if (! connected) {
    if (! warmup && digitalRead(POWER_PIN)) {
      powerDown();
    }
    myLogger._mode = 0;
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
  }

  //check if still connected
  if (connected && digitalRead(CONNECTED_PIN)) {
    client.flush();
    client.end();
    connected=0;
  }
  //Connected pin is pulled to GND
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    adjust_OSCCAL();
    client.begin(38400);
  }

}

