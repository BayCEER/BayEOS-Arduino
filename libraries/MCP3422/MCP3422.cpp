#include "../Wire/Wire.h"
#include "MCP3422.h"

MCP3422::MCP3422()
{
  Wire.begin();
}


void MCP3422::readOutputRegister(void)
{
  byte out_bytes = 0;
	byte ttt = 0;

  Wire.beginTransmission(MCP3422_ADR);
  Wire.endTransmission();

  if (resolution<3) out_bytes = 3;
  else out_bytes = 4;

  Wire.requestFrom(MCP3422_ADR, (int)out_bytes);
  while(Wire.available()<out_bytes) ;
  for(int i=0; i<out_bytes; i++)
  {
    output_bcd[i]=Wire.read();
  }
}

	// Parameter:
	// rdy	-	ready bit ->	in continuous mode has no effect
											// in one-shot mode	rdy=0 -> no efect
																				// rdy=1 -> initiate a new conversion
	// ch - channel selection	ch = 0 	->	channel 1
							// ch = 1  ->	channel	2
	// mode -	mode = 0 -> continuous mode: the ADC performs data conversion continuously
			// mode = 1 -> one-shot mode:	the ADC performs a single conversion
																			// an enters standby mode
	// rate -	sample rate selection:	rate = 0 240 SPS 		- 12-bits conversion
																	// rate = 1  60 SPS 		- 14-bits conversion
																	// rate = 2  15 SPS 		-	16-bits conversion
																	// rate = 3   3.75 SPS -	18-bits conversion
	// gain -	amplifier gain selection:	
					// gain = 0  ->  x1 gain
					// gain = 1  ->  x2 gain
					// gain = 2  ->  x4 gain
					// gain = 3  ->  x8 gain
void MCP3422::setConf(byte rdy, byte ch, byte mode, byte rate, byte gain)
{
	byte conf = ((B00000001 & rdy)  << 7)		|
							((B00000011 & ch)   << 5)		|
							((B00000001 & mode)	<< 4)		|
							((B00000011 & rate)	<< 2)		|
							 (B00000011 & gain);
	
  setConf(conf);
}

void MCP3422::setConf(byte conf)
{
  resolution = (conf & B0001100) >> 2;
  gain = (conf & B00000011);  
  Wire.beginTransmission(MCP3422_ADR);
  Wire.write(conf);
  Wire.endTransmission();
}


float MCP3422::getData(void)
{
  float lsb = 0.0;
  boolean  readOK = false;

  for(byte ii=0; ii<5; ii++) {
    readOutputRegister();
    if (resolution==3) {
      if ((output_bcd[3]&B10000000) == 0) {readOK = true; break;}  
    }
    else
      if ((output_bcd[2]&B10000000) == 0) {readOK = true; break;} 
  }

  if (!readOK) return 100.0; 

  zahl = 0;
  zahl = output_bcd[0];

  switch(resolution){
    case 0: zahl &= B00001111; break;
    case 1: zahl &= B00111111; break;
    case 2: break;
    case 3: zahl &= B00000011; break;
  }

  zahl = (zahl <<8);
  zahl += output_bcd[1];

  if (resolution ==3)  {
    zahl = (zahl<<8);
    zahl += output_bcd[2];
  }

  lsb = 2*2.048 / pow(2,12+resolution*2);

  if(output_bcd[0] & B10000000) {
    zahl = ~zahl + 1;
    switch(resolution){
      case 0: zahl &= 0x00000fff; break;
      case 1: zahl &= 0x00003fff; break;
      case 2: zahl &= 0x0000ffff; break;
      case 3: zahl &= 0x0003ffff; break;
    }
    return -((zahl*lsb)/pow(2, gain));
  }
  else return (zahl*lsb)/pow(2, gain);
}





