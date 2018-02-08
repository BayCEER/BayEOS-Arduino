/*
   Sketch for scale auto temperature calibration

   Calibration starts with:
   a) Holding ZERO + pressing SCALE -> zero temp calibration
   b) Holding SCALE + pressing ZERO -> weight temp calibration

   Calibration is stopped by pressing either ZERO or SCALE

   Active calibration process is indicated by flashing red LED
   Start of calibraton is indicated by red + green LED
   Successfull calibration by green. Error by red


*/

/*
   Define your calibraton weight,
   zero and weight adc readings

   if adc reading does not match predefined values
   no calibration is done.
*/
#define SLOPE_WEIGHT 1467.0
#define ADC_ZERO -70000L
#define ADC_WEIGHT -385000L


#include <BayEOSBuffer.h>
#include <BayDebug.h>
#include <HX711_NTC.h>
#include <EEPROM.h>

uint8_t dout[] = {A5};
uint8_t sck = A4;
HX711Array scale;
HX711_NTC cal(scale);


BayDebug client(Serial);
#define SAMPLING_INT 8
#define ACTION_COUNT 1
#include <LowCurrentBoard.h>

void setup(void)
{
  initLCB();
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(20);
  scale.begin(dout, 1, sck); //start HX711Array with 1 ADCs
  cal.init(ADC_ZERO, ADC_WEIGHT, SLOPE_WEIGHT); //init Calibration with default ADC for zero and weight
  //cal.reset(); //Reset EEPROM calibration data
  cal.printCalData();
  scale.power_down();
  startLCB();
}


void loop() {
  float temp;

  if (HX711_NTC_mode) {
    cal.handleINT();
  }

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    if (cal.cal_mode) {
      temp = cal.readNTC();
      uint8_t res=cal.single_cal(temp, 0);
      if(res){
        Serial.print("Cal failed - Error: ");
        if(res==2) Serial.println("not stable");
        else {
          Serial.println("not in range");
          Serial.print("ADC: ");
          delay(100);
          scale.power_up();
          Serial.println(scale.read_average(5));
          scale.power_down();
          Serial.print("Expecting: ");
          if(cal.cal_mode==2) Serial.println(ADC_ZERO);
          else Serial.println(ADC_WEIGHT);
        }
      }
      delay(50);
    }
  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    if (cal.cal_mode) {
      digitalWrite(LED_RED, HIGH);
      delay(20);
      digitalWrite(LED_RED, LOW);
    }
  }

  sleepLCB();
}
