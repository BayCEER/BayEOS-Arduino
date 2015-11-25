/****************************************************************
 * 
 * Sketch for MLX90614 connected to low power Arduino
 *
 ***************************************************************/ 
#define SAMPLING_INTTICKS 512
#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
#define RF24CHANNEL 0x71
#define SERIALDEBUG 0

#include <BayEOSBuffer.h>
#include <Wire.h>
#include <RTClib.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <Sleep.h>
#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>
#include <MLX90614.h>

volatile uint16_t ticks; //16 ticks per second
volatile uint8_t action;
#define WAKEUP_MASK 0x2
#define SEND_MASK 0x4
#define RESEND_MASK 0x8

MLX90614 mlx = MLX90614();

RTC_Timer2 myRTC;
ISR(TIMER2_OVF_vect){
  ticks++;
  if((ticks%SAMPLING_INTTICKS)==(SAMPLING_INTTICKS-2))
     action|=WAKEUP_MASK;
  if((ticks % 16)==0){
    myRTC._seconds += 1; 
    switch(ticks%SAMPLING_INTTICKS){
      case 0:
        action|=SEND_MASK;
        break;
      default:
        action|=RESEND_MASK;
    }
  }
}
BayRF24 client=BayRF24(9,10);
BayEOSBufferEEPROM myBuffer;

float bat;




/* Read battery voltage */
void readVoltage(){
    analogReference(INTERNAL);
    pinMode(A3,OUTPUT);
    digitalWrite(A3,HIGH);
    bat=1.1*320/100/1023*analogRead(A0);
    digitalWrite(A3,LOW);
    pinMode(A3,INPUT);
    analogReference(DEFAULT);
}

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  Wire.begin();
  client.init(RF24ADDRESS,RF24CHANNEL);
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer);
  
  readVoltage();
  mlx.enterSleepMode();
}

void loop()
{
  if(action & WAKEUP_MASK){
    action&= ~WAKEUP_MASK;
    //Wakeup without delay!
    //To measure we need a delay of at least 100ms
    //We pass the time while sleeping!
    mlx.exitSleepMode(0);
   
  }
 // Measure and send 
  if(action & SEND_MASK){
    action&= ~SEND_MASK;
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    client.addChannelValue(bat);
    client.addChannelValue(mlx.readAmbientTempC()); 
    client.addChannelValue(mlx.readObjectTempC()); 
    client.sendOrBuffer();
    //Read battery voltage _after_ long uptime!!!
    readVoltage();
    mlx.enterSleepMode();
   
  } 
  
  // Resend from Buffer
  if(action & RESEND_MASK){
    action&= ~RESEND_MASK;
    client.sendFromBuffer();
  }

  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}

