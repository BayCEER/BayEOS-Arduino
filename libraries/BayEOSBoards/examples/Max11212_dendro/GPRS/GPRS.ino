/*******************************************************
  Example Sketch 
  
  The sketch performes a measurement at a frequency of SAMPLING_INT
  It stores data an flash and tries to send the data, after collecting
  min. SEND_COUNT samples.
  When battery voltages is to low (below MIN_VOLTAGE), no data is send.

  Channels:
  1. CPU-Time [ms]
  2. Battery Voltage [V]
  3-8 Dendrometer-Readings [mm]
  9-12 Dallas-Readings [°C]  
*******************************************************/



// Divider resistors for battery voltage
#define DENDRO_LENGTH 10.0
#define BAT_DIVIDER (100.0 + 100.0) / 100.0
#define SAMPLING_INT 60
#define SEND_COUNT 60   /*collect 60 measurements before send... */
#define MIN_VOLTAGE 0 /*minimum voltage for send operation */
// SIM800-Config string. -- new Library
// Gateway-url|login|password|Origin (== Board unique identifier)|apn of sim-card|apn-user|apn-pw|PIN
#define SIM800_CONFIG "http://132.180.112.128/gateway/frame/saveFlat|import@IT|import|MyGPRS-Dendro|iot.1nce.net||||"

//END user configruation
//**********************************************


#include <BayEOSBufferSPIFlash.h>
unsigned long last_sent;

#include <BaySIM800.h>
BaySIM800 client = BaySIM800(Serial);

SPIFlash flash(8);              //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer;  //BayEOS Buffer


#include <MAX11212Board.h>
#include <DS18B20.h>
DS18B20 ds1(2, 0, 1);
DS18B20 ds2(3, 0, 1);
DS18B20 ds3(4, 0, 1);
DS18B20 ds4(6, 0, 1);
float temp;

// Some settings and functions for LowCurrentBoard
#include <LowCurrentBoard.h>



uint16_t measurements = SEND_COUNT;

void setup() {
  ds1.setAllAddr();  //Search for new or removed sensors
  ds2.setAllAddr();
  ds3.setAllAddr();
  ds4.setAllAddr();
  ds1.setResolution(12);
  ds2.setResolution(12);
  ds3.setResolution(12);
  ds4.setResolution(12);

  initLCB();
  initMAX11212();
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash);                               //This will restore old pointers
  myBuffer.skip();                                    //This will skip unsent frames
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);       //use the rtc clock but relative
  client.setBuffer(myBuffer);                         //connect the buffer to the transport client
  digitalWrite(POWER_PIN, HIGH);                      //power up GPRS-Modem
  startLCB();                                         //some settings and blink three times
  client.readConfigFromStringPGM(PSTR(SIM800_CONFIG));  //read GPRS config into RAM
  adjust_OSCCAL();                                   //tune clock of ATMega to make serial communication more stable
  delayLCB(1000);
  blinkLED(client.begin(38400) + 1);                  //start the GPRS-Modem (e.g. connect to network)
  //one time blinking indicates all ok - more denotes an error. For details look at function definition
  delay(2000);
  blinkLED(client.sendMessage("Board started") + 1);  //send a message to the gateway.
  //one time blinking indicates success - more denotes an error. For details look at the function definition
  Serial.end();                  //Stop Serial (avoids power leakage via TX Pin)
  digitalWrite(POWER_PIN, LOW);  //power down GPRS-Modem
  delay(2000);
}



void loop() {
  if (ISSET_ACTION(0)) {  //will be set by timer2 interrupt each SAMPING_INT seconds
    UNSET_ACTION(0);      //clear the flag
    ds1.t_conversion();   //Start T-conversion
    ds2.t_conversion();   //Start T-conversion
    ds3.t_conversion();   //Start T-conversion
    ds4.t_conversion();   //Start T-conversion
    delayLCB(900);
    digitalWrite(POWER_PIN, HIGH);  //power up modem AND power divider (for battery voltage read)
    analogReference(DEFAULT);
    delayLCB(100);
    float bat_voltage = 3.3 * BAT_DIVIDER / 1023 * analogRead(A7);  //Read battery voltage
    digitalWrite(POWER_PIN, LOW); 
    client.startDataFrame();                                        //Start the data frame with float values
    client.addChannelValue(millis());                               //add uptime (ms) als 4 byte float
    client.addChannelValue(bat_voltage);                            //add battery voltage

    adc.read(1);  //read with calibration
    adc.read();   //read once without calibration
    for (uint8_t i = 0; i < 6; i++) {
      client.addChannelValue(DENDRO_LENGTH * readADC(i, 5) / 131071);
    }


    ds1.readChannel(1, &temp);
    client.addChannelValue(temp);
    ds2.readChannel(1, &temp);
    client.addChannelValue(temp);
    ds3.readChannel(1, &temp);
    client.addChannelValue(temp);
    ds4.readChannel(1, &temp);
    client.addChannelValue(temp);
    

    client.writeToBuffer();                                        //Save data to flash
    measurements++;                                                //increase measurement count
    if (measurements >= SEND_COUNT & bat_voltage > MIN_VOLTAGE) {  //time to send data
      digitalWrite(POWER_PIN, HIGH);                               //power up modem
      adjust_OSCCAL();
      delayLCB(1000);
      uint8_t tx_res = client.begin(38400);
      blinkLED(tx_res + 1);  //connect to network
      while (!tx_res && myBuffer.available() && !ISSET_ACTION(0)) {
        tx_res = client.sendMultiFromBuffer(3000);  //send 3000 bytes from flash storage
        blinkLED(tx_res + 1);
      }
      if (!myBuffer.available()) measurements = 0;  //all data sent from flash storage
      Serial.end();
      digitalWrite(POWER_PIN, LOW);  //power down modem
    }
  }
  sleepLCB();  //board stops execution here until timer2 wakes up the board again (typ. 16 per second)
  //Most time the boards just wakes up, runs the timer2-ISR and as there is nothing to do just
  //runs directly into sleepLCB() again.
}
