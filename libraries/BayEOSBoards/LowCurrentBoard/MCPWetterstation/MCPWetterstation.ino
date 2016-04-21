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

#define TICKS_PER_SECOND 16
#define RAINGAUGE_LAGTICKS 2
#define SAMPLING_INT 32
#define WITHDALLAS 0
#define WITH_MLX 1
#define WITHRAINGAUGE 1
//#define RF24ADDRESS 0x45c431ae12LL
//#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
//#define RF24ADDRESS 0x45c431ae96LL
#define RF24ADDRESS 0x45c431aeabLL
//#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x72

//Set this to 1 to get BayDebug Output!
#define SKETCH_DEBUG 0


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
#else
  client.init(RF24ADDRESS, RF24CHANNEL);
#endif
  myBuffer.init(0x50, 65536L, 0); //NO flush!!
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 100); //use skip!
  initLCB(); //init time2
  readBatLCB();
  startLCB();
#if WITH_MLX
  mlx.enterSleepMode();  
#endif


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
    client.startDataFrame(BayEOS_Int16le);
    client.addChannelValue(millis());
    client.addChannelValue(1000 * batLCB);
    hum = SHT2x.GetHumidity();
    temp = SHT2x.GetTemperature();
    SHT2x.reset();
    client.addChannelValue(100 * temp);
    client.addChannelValue(100 * hum);
    mcp342x.setConf(addr, 1, 3, mode, rate, gain);
    delayLCB(350);
    client.addChannelValue((100000.0 * mcp342x.getData(addr)));
#if WITHRAINGAUGE
    client.addChannelValue(100 * rain_count / SAMPLING_INT);
    rain_count = 0;
#endif

#if WITH_MLX
    mlx.exitSleepMode(0);
    delayLCB(300);
    client.addChannelValue(100 * mlx.readAmbientTempC());
    client.addChannelValue(100 * mlx.readObjectTempC());
    mlx.enterSleepMode();
#endif

    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();

  }

  // Resend from Buffer
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);

#if SERIALDEBUG
    Serial.println(myBuffer.available());
    delay(20);
#endif

    client.sendFromBuffer();
  }
#if SKETCH_DEBUG
  delay(50);
#else
  sleepLCB();
#endif
}


