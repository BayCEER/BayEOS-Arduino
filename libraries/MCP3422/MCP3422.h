/*
  MCP3422.h - library for MCP3422.h AD-Controller
*/

// ensure this library description is only included once
#ifndef MCP3422_h
#define MCP3422_h

// include types & constants of Wiring core API
#include <Arduino.h>

#define MCP3422_ADR B1101000 

// library interface description
class MCP3422
{
  public:
    MCP3422();
    float getData(void);
    void setConf(byte conf);
		void setConf(byte rdy, byte ch, byte mode, byte rate, byte gain);
  private:
		void print_binary(byte);
//    byte confByte;
    byte resolution;
    byte gain;
    long zahl;
    byte output_bcd[5];

    void readOutputRegister(void);
    void saveData(void);
};

#endif
 

