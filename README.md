# BayEOS-Arduino-lib
This is the repository for [BayEOS](http://www.bayceer.uni-bayreuth.de/bayeos/) Arduino libraries. The libraries are designed to compile with Arduino IDE 2.x

## Installation
1. Copy the URL (https://raw.githubusercontent.com/BayCEER/BayEOS-Arduino/master/package_BayEOS_index.json) to your additional boardmanager-URLs in your arduino IDE preferences (file/preferences)
2. Install the package via tools/Board/Boardmanager
3. Install the following libraries via sketch/Libraries/Librarymanager
	- RF24 by TMRh20
	- OneWire by Jim Studt, Tom Pollard ...
  

This library relays on some third party libraries (see below). To ease up installation
these libraries have been included into this repository.

## Usage
A typical example sketch for sending sensordata
look like this. The sketch is optimized for very low power comsumption and unstable
networks.

```c++
    #define SAMPLING_INT 32
    #define RF24CHANNEL 0x66
    #define RF24ADDRESS 0x45c431ae12LL
    #define WITH_CHECKSUM 1

    #include <BayEOSBufferSPIFlash.h>
    SPIFlash flash(8);
    BayEOSBufferSPIFlash myBuffer;

    #include <BayRF24.h>
    BayRF24 client = BayRF24(9, 10);

    #include <LowCurrentBoard.h>

    void setup()
    {
      client.init(RF24ADDRESS, RF24CHANNEL);
      myBuffer.init(flash); //This will restore old pointers
      myBuffer.skip(); //This will move read pointer to write pointer
      myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS); //use RTC but relativ -- delay only
      client.setBuffer(myBuffer, 100); //use skip!
      initLCB(); //init time2
      readBatLCB();
      startLCB(); //Board blinks 3x
    }


    void loop() {
      if (ISSET_ACTION(0)) {
        UNSET_ACTION(0);
        //eg measurement
        //Note we start a INT-Dataframe here
        //Only Values between -32768 and +32767 can be sent
        client.startDataFrame(BayEOS_Int16le, WITH_CHECKSUM);
        client.addChannelValue(millis());
        client.addChannelValue(1000 * batLCB);
    #if WITH_CHECKSUM
        client.addChecksum();
    #endif
        sendOrBufferLCB(); //Board blinks 1 time on success and 2 times on error
        //Read battery voltage _after_ long uptime!!!
        readBatLCB();
      }

      if (ISSET_ACTION(7)) {
        UNSET_ACTION(7);
        client.sendFromBuffer(); //send data from buffer
      }
      sleepLCB(); //most time board sleeps here
}
```

## Content
### Included unmodified third party libraries 
1. [Base64](https://github.com/adamvr/arduino-base64/)
2. [SPIFlash](https://github.com/Marzogh/SPIFlash/)

### Included modified third party libraries
1. [I2C_EEPROM](https://github.com/RobTillaart/Arduino/tree/master/libraries/I2C_EEPROM) (additional constructor)
2. [SDI-12](https://github.com/EnviroDIY/Arduino-SDI-12/tree/master) (adjusted to run with timer1)

### BayEOS libraries

#### BayEOS
Provides all transport related classes (e.g. BayRF24, BaySerial, BaySim800...)
BayXBee includes a modified version of [XBee](https://github.com/andrewrapp/xbee-arduino)

#### BayEOSBuffer
Classes to store data locally

#### BayEOSLogger
Classes and examples to build logger

#### BayEOSBoards
Provides header files and examples for boards and shield developed by BayCEER

#### BayEOSRTClib
RTC classes required for BayEOSBuffer

#### BayEOSSensors
Classes and examples for sensors

