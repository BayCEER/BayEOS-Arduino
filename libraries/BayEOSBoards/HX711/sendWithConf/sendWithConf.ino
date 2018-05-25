/***************************************
 * Example Sketch from BayEOS LP HX711 Board
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

//We use routed origin frames in this sketch
//so RF24ADDRESS can stay unchanged for all scales
//Change boxname
//Do not use long names!! RF24 is limited to 32 byte!!!
//e.g: [DF][long][CF][RO][l][NAME][DF][T][temp][adc][weight][cs]
//      1    4    1   1   1   3    1   1   4     4    4      2 = 27
const char boxname[]="W0";
#define RF24CHANNEL 0x37
#define RF24ADDRESS 0x45c431ae12LL

#define BAT_MULTIPLIER 3.3*200.0/100.0/1023

uint8_t dout[] = {6, 4};
uint8_t sck = 3;
long adc[2];
float temp0, temp1;
char origin[6];

HX711Array scale;
NTC_HX711 ntc(scale, A3, 2*470000, 3.0); //adjust resistor values
Scale4PointCal cal0;
Scale4PointCal cal1(28); //One Config needs 28 Byte in EEPROM - so second has a offest of 28

BayRF24 client(9, 10);
SPIFlash flash(8); //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer; //BayEOS Buffer


volatile uint8_t int0_flag = 0;
void isr_int0(void) {
  int0_flag = 1;
}

#define ACTION_COUNT 1
#define SAMPLING_INT 64
#include <LowCurrentBoard.h>

void setup(void) {
  pinMode(POWER_PIN,OUTPUT);
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  myBuffer.skip(); //This will skip unsent frames
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer);
  initLCB(); //init time2
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), isr_int0, FALLING);

  scale.begin(dout, 2, sck); //start HX711Array with 1 ADCs
  cal0.readConf();
  cal1.readConf();
  int0_flag = 0;
  strcpy(origin,boxname);
  startLCB();

}


void loop(void) {
  if (int0_flag) {
    digitalWrite(LED_BUILTIN, HIGH);
    ntc.readResistance();
    temp0 = ntc.getTemp(0);
    temp1 = ntc.getTemp(1);

    scale.power_up();
    scale.read_average(adc);
    scale.power_down();

    cal0.setTare(adc[0], temp0);
    cal1.setTare(adc[1], temp1);
    digitalWrite(LED_BUILTIN, LOW);
    int0_flag = 0;
  }

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);

    ntc.readResistance();
    temp0 = ntc.getTemp(0);
    temp1 = ntc.getTemp(1);

    scale.power_up();
    scale.read_average(adc);
    digitalWrite(POWER_PIN,HIGH);
    origin[strlen(boxname)]=0;
    client.startDataFrameWithOrigin(BayEOS_WithoutOffsetFloat32le, origin, 1, 1); //with Checksum and routed!
    client.addChannelValue(millis());
    client.addChannelValue(BAT_MULTIPLIER*analogRead(A7));
    client.addChecksum();
    digitalWrite(POWER_PIN,LOW);
    scale.power_down();
    sendOrBufferLCB();

    
    origin[strlen(boxname)]='a';
    client.startDataFrameWithOrigin(BayEOS_WithoutOffsetFloat32le, origin, 1, 1); //with Checksum and routed!
    client.addChannelValue(temp0);
    client.addChannelValue(adc[0]);
    client.addChannelValue(cal0.getWeight(adc[0], temp0));
    client.addChecksum();
    client.sendOrBuffer();
 
    origin[strlen(boxname)]='b';
    client.startDataFrameWithOrigin(BayEOS_WithoutOffsetFloat32le, origin, 1, 1); //with Checksum and routed!
    client.addChannelValue(temp1);
    client.addChannelValue(adc[1]);
    client.addChannelValue(cal1.getWeight(adc[1], temp1));
    client.addChecksum();
    client.sendOrBuffer();
  }

  
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}


