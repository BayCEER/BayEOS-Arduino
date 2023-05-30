/*
 * Calibration Sketch
 *
 */

#include <HX711Array.h>
#include <Sleep.h>

//Make sure, that you have choosen the right board!
#define HX711_DOUT 6
#define HX711_SCK 3
uint8_t dout[] = { HX711_DOUT };
uint8_t sck = HX711_SCK;

HX711_4PointCal scale;

float sw = 3.00;
float t[] = { 10.0, 20.0 };
long cal_adc[] = { -68137L, -73803, -378845L, -384611L };
float t_coef = 0;


volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}


void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  printHelp();
  Serial.flush();
  Sleep.setupTimer2(2);  //init timer2 to 1/16 sec
  scale.set_no_sleep(1);
  scale.begin(dout, 1, sck);  //start HX711Array with one ADCs
  scale.power_up();
}

String s;
long adc;
char c = 0;
void loop() {
  Serial.flush();
  char cc = 0;
  if (Serial.available()) {
    cc = Serial.read();
    if ((cc >= 'a' && cc <= 'z') || (cc >= 'A' && cc <= 'Z') || (cc >= '0' && cc <= '1')) c = cc;
  }

  switch (c) {
    case 'h':
      printHelp();
      c = '0';
      break;
    case '1':
    case 't':
      adc = scale.read_average(10);
      if (c == 't') {
        scale.setTare(adc, 20);
        c = '1';
      }
      Serial.print("ADC: ");
      Serial.print(adc);
      Serial.print(" -- Weight: ");
      Serial.println(scale.getWeight(adc, 20));
      break;
    case 'z':
      Serial.print("Reading Zero ADC...");
      Serial.flush();
      adc = scale.read_average(50);
      Serial.println(adc);
      cal_adc[1] = adc;
      cal_adc[0] = cal_adc[1] - t_coef * 10;

      c = '0';
      break;
    case 'w':
      Serial.print("Reading Weight ADC...");
      Serial.flush();
      adc = scale.read_average(50);
      Serial.println(adc);
      cal_adc[3] = adc;
      cal_adc[2] = cal_adc[3] - t_coef * 10;

      c = '0';
      break;
    case 'r':
      Serial.println("Reading Conf...");
      scale.readConf();
      c = '0';
      break;
    case 's':
      Serial.println("Saving Conf...");
      scale.setConf(sw, t, cal_adc);
      scale.saveConf();
      c = '0';
      break;
    case 'p':
      scale.printConf();
      c = '0';
      break;
    case 'c':
      while (Serial.available()) {
        delay(1);
        Serial.read();
      }
      Serial.println("Please enter calibration weight");
      while (Serial.available() < 3) {}
      s = Serial.readString();
      s.trim();
      sw = s.toFloat();
      Serial.print("Calibration weight: ");
      Serial.println(sw);
      c = '0';
      break;
    case 'T':
      while (Serial.available()) {
        delay(1);
        Serial.read();
      }
      Serial.println("Please enter ");
      while (Serial.available() < 3) {}
      s = Serial.readString();
      s.trim();
      t_coef = s.toFloat();
      Serial.print("Calibration weight: T-coefficient ADC/°C");
      Serial.println(t_coef);
      cal_adc[2] = cal_adc[3] - t_coef * 10;
      cal_adc[0] = cal_adc[1] - t_coef * 10;
      c = '0';
      break;
    case '0':
      break;
  }
}


void printHelp(void) {
  Serial.println("Commands:");
  Serial.println("z: Zero calibration");
  Serial.println("w: Weight calibration");
  Serial.println("c: Set calibration weight");
  Serial.println("T: Set T-coefficient ADC/°C");
  Serial.println("p: Print calibration");
  Serial.println("r: Read calibration from EEPROM");
  Serial.println("s: Save calibration to EEPROM");
  Serial.println("t: Tare");
  Serial.println("1: Continuous readings");
  Serial.println("0: Stop readings");
  Serial.println("h: Print help");
}