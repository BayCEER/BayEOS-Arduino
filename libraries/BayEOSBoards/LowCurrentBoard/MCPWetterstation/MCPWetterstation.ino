/****************************************************************

   Sketch for simple and cheap weather station

   with
   air temperature - air moisture (SHT21)
   rain gauge/wind count
   photo diode on mcp3424


 ***************************************************************/
#define DALLAS_PIN 4
#define POWER_PIN 7
#define LED_PIN 5

#define RAINGAUGE_LAGTICKS 2
#define SAMPLING_INT 32
#define WITHDALLAS 0
#define WITH_MLX 1
#define WITHWINDSPEED 0

#define NRF24_PIPE 2
#define RF24CHANNEL 0x47
#define CHECKSUM_FRAMES 1

#if NRF24_PIPE == 0
#define RF24ADDRESS 0x45c431ae12LL
#elif NRF24_PIPE == 1
#define RF24ADDRESS 0x45c431ae24LL
#elif NRF24_PIPE == 2
#define RF24ADDRESS 0x45c431ae48LL
#elif NRF24_PIPE == 3
#define RF24ADDRESS 0x45c431ae96LL
#elif NRF24_PIPE == 4
#define RF24ADDRESS 0x45c431aeabLL
#elif NRF24_PIPE == 5
#define RF24ADDRESS 0x45c431aebfLL
#endif

//Set this to 1 to get BayDebug Output!
#define SKETCH_DEBUG 0

#if WITHWINDSPEED
#define TICKS_PER_SECOND 128
#else
#define TICKS_PER_SECOND 16
#endif



#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <BayEOSBuffer.h>
#include <Wire.h>
#include <RTClib.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <Sleep.h>
#include <SHT2xSleep.h>
#include <BayEOS.h>
#include <MCP342x.h>

#if WITH_MLX
#include <MLX90614.h>
MLX90614 mlx = MLX90614();

void switch_on_mlx(void) {
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
}
void switch_off_mlx(void) {
  digitalWrite(4, HIGH);
  pinMode(4, INPUT);
}
#endif

#if SKETCH_DEBUG
#include <BayDebug.h>
BayDebug client(Serial);
#else
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);
#endif
BayEOSBufferEEPROM myBuffer;


//include some functions for low current board
//expects BayEOS-Client to be called "client"

#include <LowCurrentBoard.h>


#if WITHWINDSPEED
volatile uint16_t wind_count = 0;
volatile uint8_t wind_event = 0;
volatile uint16_t wind_event_ticks;
volatile uint16_t min_wind_ticks = 65535;
void wind_isr(void) {
  if ((ticks - wind_event_ticks) > 4) {
    if ((ticks - wind_event_ticks) < min_wind_ticks)
      min_wind_ticks = ticks - wind_event_ticks;
    wind_count++;
    wind_event = 1;
    wind_event_ticks = ticks;
  }
}
#endif

float temp, hum;

MCP342x mcp342x = MCP342x();
const byte addr = 0;
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 3; //0-3: 12bit ... 18bit
const uint8_t gain = 2; //max Voltage: 0,512 Volt

void setup()
{

  Wire.begin();
#if SKETCH_DEBUG
  client.begin(9600, 1);
  Serial.println("Starting");
  delay(50);
#else
  client.init(RF24ADDRESS, RF24CHANNEL);
#endif
  myBuffer.init(0x50, 65536L, 0); //NO flush!!
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 100); //use skip!
  initLCB(); //init time2
  readBatLCB();
#if WITH_MLX
   mlx.enterSleepMode();
#endif
#if WITHWINDSPEED
  attachInterrupt(0, wind_isr, RISING);
  digitalWrite(2, LOW);
  wind_count = 0;
  wind_event = 0;
#endif
#if SKETCH_DEBUG
  Serial.println("End setup");
  delay(50);
#endif
  startLCB(); 


}

void loop()
{
  handleRtcLCB();

#if WITHDALLAS
  //Do conversion 32 ticks (2sec) bevor sampling!
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    ds.t_conversion();
  }
  if (ISSET_ACTION(1)) {
    UNSET_ACTION(1);
    readAndSendDallasLCB();
    readBatLCB();
  }
#endif

#if WITHRAINGAUGE
  handleRainEventLCB();
#endif


  // Measure and send
  if (ISSET_ACTION(2)) {
    UNSET_ACTION(2);
    client.startDataFrame(BayEOS_Int16le,CHECKSUM_FRAMES);
    client.addChannelValue(millis());
    client.addChannelValue(1000 * batLCB);
#if WITH_MLX
    switch_on_mlx();
    mlx.exitSleepMode(0);
    hum = SHT2x.GetHumidity();
#endif
    temp = SHT2x.GetTemperature();
    hum = SHT2x.GetHumidity();
    SHT2x.reset();
#if SKETCH_DEBUG
    Serial.println("END SHT");
    delay(50);
#endif
    client.addChannelValue(100 * temp);
    client.addChannelValue(100 * hum);
    mcp342x.setConf(addr, 1, 3, mode, rate, gain);
    delayLCB(350);
    client.addChannelValue((100000.0 * mcp342x.getData(addr)));
#if SKETCH_DEBUG
    Serial.println("END MCP");
    delay(50);
#endif
#if WITHWINDSPEED
    client.addChannelValue(100 * wind_count / SAMPLING_INT);
    noInterrupts();
    wind_count = 0;
    interrupts();
#endif

#if WITH_MLX
    client.addChannelValue(100 * mlx.readAmbientTempC());
    client.addChannelValue(100 * mlx.readObjectTempC());
    mlx.enterSleepMode();
#endif
#if CHECKSUM_FRAMES
  client.addChecksum();
#endif

    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();
#if WITH_MLX
    switch_off_mlx();
#endif
  }

  // Resend from Buffer
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);

#if SERIALDEBUG
    Serial.println(myBuffer.available());
    delay(20);
#endif
    if (myBuffer.available()) {

#if WITH_MLX
      switch_on_mlx();
#endif
      client.sendFromBuffer();
#if WITH_MLX
      switch_off_mlx();
#endif

    }
  }
#if SKETCH_DEBUG
  delay(50);
#else
  sleepLCB();
#endif
}


