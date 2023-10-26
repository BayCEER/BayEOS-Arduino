/*******************************************************
  Example Sketch 
  
  The sketch performes a measurement at a frequency of SAMPLING_INT
  It stores data an flash and tries to send the data, after collecting
  min. SEND_COUNT samples.
  When battery voltages is to low (below MIN_VOLTAGE), no data is send.


*******************************************************/



// Divider resistors for battery voltage
#define BAT_DIVIDER (100.0+100.0)/100.0
#define SAMPLING_INT 60
#define SEND_COUNT 60 /*collect 60 measurements before send... */
#define MIN_VOLTAGE 3.8 /*minimum voltage for send operation */
#define NETWORK_TIME 1 /*use network time*/
#define BUFFER_RESET_ON_STARTUP 1 /*only set to 0 with NETWORK_TIME*/
// SIM800-Config string. -- new Library
// Gateway-url|login|password|Origin (== Board unique identifier)|apn of sim-card|apn-user|apn-pw|PIN
#define SIM800_CONFIG "http://132.180.112.128/gateway/frame/saveFlat|import@IT|import|MyGPRS-Board|iot.1nce.net||||"

// GPRS-Config string. -- old TCP-Library
// Gateway-IP|PORT|path on server|login|password|Origin (== Board unique identifier)|apn of sim-card|apn-user|apn-pw|PIN
//#define GPRS_CONFIG "132.180.112.128|80|gateway/frame/saveFlat|import@IT|import|MyGPRS-Board|iot.1nce.net||||"

//END user configruation
//**********************************************

#if defined(GPRS_CONFIG)
#include <BayTCPSim900.h>
BayGPRS client = BayGPRS(Serial, 0);
#else
#include <BaySIM800.h>
BaySIM800 client = BaySIM800(Serial);
#endif


#include <BayEOSBufferSPIFlash.h>
unsigned long last_sent;


SPIFlash flash(8); //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer; //BayEOS Buffer

// Some settings and functions for LowCurrentBoard
#include <LowCurrentBoard.h>



uint16_t measurements = SEND_COUNT;

void setup()
{
  initLCB();
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash); //This will restore old pointers
#if BUFFER_RESET_ON_STARTUP
  myBuffer.skip(); //This will skip unsent frames
#endif
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS); //use the rtc clock but relative
  client.setBuffer(myBuffer); //connect the buffer to the transport client
  digitalWrite(POWER_PIN, HIGH); //power up GPRS-Modem
  startLCB(); //some settings and blink three times
#if defined(GPRS_CONFIG)
  client.readConfigFromStringPGM(PSTR(GPRS_CONFIG)); //read GPRS config into RAM
#else
  client.readConfigFromStringPGM(PSTR(SIM800_CONFIG)); //read GPRS config into RAM
#endif
  adjust_OSCCAL(); //tune clock of ATMega to make serial communication more stable
  delayLCB(1000);
  blinkLED(client.begin(38400) + 1); //start the GPRS-Modem (e.g. connect to network)
  //one time blinking indicates all ok - more denotes an error. For details look at function definition
  delay(2000);
  blinkLED(client.sendMessage("Board started") + 1);  //send a message to the gateway.
  //one time blinking indicates success - more denotes an error. For details look at the function definition
#if NETWORK_TIME
  while (true) {
    unsigned long time = client.now().get();
    if (time > 3600L*24*365*10) {
      myRTC.adjust(time);
      break;
    }
    delay(2000);
    blinkLED(2);
  }
#endif

  Serial.end(); //Stop Serial (avoids power leakage via TX Pin)
  digitalWrite(POWER_PIN, LOW); //power down GPRS-Modem
  delay(2000);
}



void loop() {
  if (ISSET_ACTION(0)) { //will be set by timer2 interrupt each SAMPING_INT seconds
    UNSET_ACTION(0); //clear the flag
    digitalWrite(POWER_PIN, HIGH); //power up modem AND power divider (for battery voltage read)
    analogReference(DEFAULT);
    delayLCB(100);
    float bat_voltage = 3.3 * BAT_DIVIDER / 1023 * analogRead(A7); //Read battery voltage
    digitalWrite(POWER_PIN, LOW); //power down modem AND power divider
    client.startDataFrame(); //Start the data frame with float values
    client.addChannelValue(millis()); //add uptime (ms) als 4 byte float
    client.addChannelValue(bat_voltage); //add battery voltage

    // TODO: add more sensor measurements
     
    client.writeToBuffer(); //Save data to flash
    measurements++; //increase measurement count
    if (measurements >= SEND_COUNT & bat_voltage > MIN_VOLTAGE) { //time to send data
      digitalWrite(POWER_PIN, HIGH); //power up modem
      adjust_OSCCAL();
      delayLCB(1000);
      uint8_t tx_res = client.begin(38400);
      blinkLED(tx_res + 1);  //connect to network
      while (! tx_res && myBuffer.available()) {
        tx_res = client.sendMultiFromBuffer(3000); //send 3000 bytes from flash storage
        blinkLED(tx_res + 1);
#if NETWORK_TIME
        unsigned long time = client.now().get();
        if (time > 3600L*24*365*10) {
          myRTC.adjust(time);
        }
#endif
        if(ISSET_ACTION(0)) break;
      }
      if (! myBuffer.available()) measurements = 0; //all data sent from flash storage
      Serial.end();
      digitalWrite(POWER_PIN, LOW); //power down modem

    }
  }
  sleepLCB(); //board stops execution here until timer2 wakes up the board again (typ. 16 per second)
  //Most time the boards just wakes up, runs the timer2-ISR and as there is nothing to do just 
  //runs directly into sleepLCB() again.
}
