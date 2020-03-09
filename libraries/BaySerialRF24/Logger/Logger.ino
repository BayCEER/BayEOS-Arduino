/****************************************************************

   Example Sketch for running BayEOS-LCB with SPI-flash as logger

***************************************************************/
#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 3
#define NRF24_TRYINT 60
const uint8_t channel = 0x70;
const uint8_t adr[] = {0x46, 0xf0, 0xe3, 0xc4, 0x45};

#include <BayEOSBufferSPIFlash.h>
#include <BaySerialRF24.h>

RF24 radio(9, 10);
BaySerialRF24 client(radio, 100, 3); //wait maximum 100ms for ACK

#include <BayEOSLogger.h>
#include <Sleep.h>


#define TICKS_PER_SECOND 16
uint8_t connected = 0;

SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>



//Add your sensor measurements here!
float values[NUMBER_OF_CHANNELS ];
uint16_t count;
unsigned long last_measurement;

uint16_t current_tics, last_tics;


void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
      values[i] = 0;
    }
  }

  //Add your sensor measurements here!

  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  myLogger._bat = (1.1 * (220 + 100) / 100 / 1023 * analogRead(A7)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);

  values[1] +=connected;

  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < NUMBER_OF_CHANNELS + 1; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }
}



void setup() {
  client.init(channel, adr);
  radio.powerDown();
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 3300); //min_sampling_int = 60, LOW BAT Warning 3700mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  initLCB(); //init time2
}


unsigned long last_try = -NRF24_TRYINT;

void send_test_byte(void) {
  uint8_t test_byte[] = {XOFF};
  uint8_t res = 0;
  last_try = myRTC.get();
  if (! connected) radio.powerUp();
  radio.stopListening();
  res = radio.write(test_byte, 1);
  uint8_t curr_pa = 0;
  while (!res && curr_pa < 4) {
    radio.setPALevel((rf24_pa_dbm_e) curr_pa);
    delayMicroseconds(random(1000));
    res = radio.write(test_byte, 1);
    curr_pa++;
  }
  if (res){
    blinkLED(0);
    client.last_activity = millis();
    radio.startListening();
    digitalWrite(LED_BUILTIN, 1);
    connected=1;
  } else {
    digitalWrite(LED_BUILTIN, 0);
    radio.powerDown();
    connected=0;
  }
}

void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;

  if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                       (myRTC.get() - last_measurement) >= SAMPLING_INT)) {
    last_measurement = myRTC.get();
    measure();
  }
  myLogger.run();

  if (! connected) {
    myLogger._mode = 0;
    //sleep until timer2 will wake us up...
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
    
   //check if receiver is present
   if((myRTC.get() - last_try) > NRF24_TRYINT) {
      send_test_byte();
      if (! connected && myLogger._logging_disabled)
        blinkLED(NRF24_TRYINT * 2);
    } 
  } else if((millis() - client.last_activity) > 30000) {
    //check if still connected
    send_test_byte();
  }

}
