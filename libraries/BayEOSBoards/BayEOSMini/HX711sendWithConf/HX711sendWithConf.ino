/***************************************
 * Example Sketch from BayEOS Mini V1.0 with HX711
 * 
 * Power Consumption:
 * Sleep-Mode: 30µA
 * Measuring:  20mA
 * 
 * 
 **************************************/
 
#include <HX711Array.h>
#include <NTC.h>
#include <Sleep.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayRF24.h>

#define RF24CHANNEL 0x13
#define RF24ADDRESS 0x45c431ae12LL

#define BAT_MULTIPLIER 3.3*200.0/100.0/1023

uint8_t dout[] = {A0};
uint8_t sck = A1;
long adc[1];
float temp0;

HX711Array scale;
NTC_HX711 ntc(scale, A3, 2*470000, 5.0); //adjust resistor values
Scale4PointCal cal0;

BayRF24 client(9, 10);
SPIFlash flash(8); //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer; //BayEOS Buffer


volatile uint8_t int0_flag = 0;
void isr_int0(void) {
  int0_flag = 1;
}

#define ACTION_COUNT 1
#define SAMPLING_INT 64

#define RESET_COUNT 1
#include <LowCurrentBoard.h>

void setup(void) {
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  myBuffer.skip(); //This will skip unsent frames
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer);
  initLCB(); //init time2
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), isr_int0, FALLING);

  scale.begin(dout, 1, sck); //start HX711Array with 1 ADCs
  cal0.readConf();
  int0_flag = 0;
  startLCB();

}


void loop(void) {
  if (int0_flag) {
    digitalWrite(LED_BUILTIN, HIGH);
    ntc.readResistance();
    temp0 = ntc.getTemp(0);

    scale.power_up();
    scale.read_average(adc);
    scale.power_down();

    cal0.setTare(adc[0], temp0);
    digitalWrite(LED_BUILTIN, LOW);
    int0_flag = 0;
  }

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);

    ntc.readResistance();
    temp0 = ntc.getTemp(0);

    scale.power_up();
    scale.read_average(adc);
    client.startDataFrame(BayEOS_WithoutOffsetFloat32le, 1); //with Checksum and routed!
    client.addChannelValue(BAT_MULTIPLIER*analogRead(A6));
    scale.power_down();
    client.addChannelValue(temp0);
    client.addChannelValue(adc[0]);
    client.addChannelValue(cal0.getWeight(adc[0], temp0));
    client.addChecksum();
    sendOrBufferLCB();

    
  }

  
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}


