# BayEOS-Arduino-lib
This is the new repository for [BayEOS](http://www.bayceer.uni-bayreuth.de/bayeos/) Arduino libraries. The libraries are designed to compile with Arduino IDE 1.6.x

## Installation
Copy all folders below the libraries (BayEOS, BayEOSBoards ..) in your Arduino libraries directory.

This library relays on some third party libraries (see below). To ease up installation
these libraries have been included into this repository.

## Usage
A typical example sketch for sending sensordata and messages to a BayEOS gateway
look like this:

    #include <Ethernet.h>
    #include <BayEOS.h>
    #include <BayTCP.h>
    #include <BayTCPEth.h>
    //Please enter a valid Mac and IP
    byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
    BayEth client;

    void setup(void){
      pinMode(10,OUTPUT);
      //Ethernet.begin(mac,ip);
      Ethernet.begin(mac);

      //BayEOS Gateway Configuration
      client.readConfigFromStringPGM(PSTR("192.168.0.1|80|gateway/frame/saveFlat|import|import|TestEth|||||"));
    }
    void loop(void){
      //Construct DataFrame
      client.startDataFrame();
      client.addChannelValue(73.43);
      client.addChannelValue(3.18);
      client.addChannelValue(millis()/1000);
      client.sendPayload();
      //Send a message
      client.sendMessage("Just a message!");
      delay(5000);
    }

## Content
### Included unmodified third party libraries
1. [OneWire](http://www.pjrc.com/teensy/td_libs_OneWire.html)
2. [RF24](https://github.com/TMRh20/RF24)
3. [SdFat](https://github.com/greiman/SdFat)
4. [UTFT](http://www.rinkydinkelectronics.com/library.php?id=51)
5. [Base64](https://github.com/adamvr/arduino-base64/)

### Included modified third party libraries
1. [iBoard](https://github.com/andykarpov/iBoardRF24) *modified*

### BayEOS libraries
#### BayEOS
Provides all transport related classes (e.g. BayRF24, BayXBee, BayEthSim900...)
BayXBee includes a modified version of [XBee](https://github.com/andrewrapp/xbee-arduino)

#### BayEOSBoards
Provides header files and examples for boards and shield developed by BayCEER

### HardwareSerialPlus
This is a modified HardwareSerial library with custom serial buffers. The library
is used to receive packages via XBee. XBee frames comprise up to 100 bytes.
Having the default Arduino serial buffer with 64 bytes often results in data loss.
