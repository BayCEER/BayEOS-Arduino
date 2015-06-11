#include "ltc1290.h"
#include <Arduino.h>

#define LTC_ACLK 1
#define LTC_CTIME 52
#define LTC_DTIME 1

LTC1290::LTC1290(uint8_t sclk,uint8_t din,uint8_t dout,uint8_t cs){
	_sclk=sclk;
	_din=din;
	_dout=dout;
	_cs=cs;
}

void LTC1290::init(void){
	pinMode(_sclk, OUTPUT);
	pinMode(_din, OUTPUT);
	pinMode(_cs, OUTPUT);
	pinMode(_dout, INPUT_PULLUP);
	digitalWrite(_cs, HIGH);
	digitalWrite(_sclk, LOW);
	digitalWrite(_din, LOW);
}

void LTC1290::read(int* res,uint8_t rep){
	if(rep>3) rep=3;
	//Variablen deklarieren
	uint8_t ch,i,j;
	digitalWrite(_cs, LOW);

	int messwert, bitwert, kanal;
	for(j=0;j<1<<rep;j++){
	//Kanal in Steuerbit wandeln
	for(ch=0;ch<=8;ch++){
	switch (ch) {
	case 0:
		kanal = 0x8EF;
		break;
	case 1:
		kanal = 0xCEF;
		break;
	case 2:
		kanal = 0x9EF;
		break;
	case 3:
		kanal = 0xDEF;
		break;
	case 4:
		kanal = 0xAEF;
		break;
	case 5:
		kanal = 0xEEF;
		break;
	case 6:
		kanal = 0xBEF;
		break;
	case 7:
		kanal = 0xFEF;
		break;
	}
		//Messwert und Bitwert auf Bit11 rücksetzen
		messwert = 0x00;
		bitwert = 0x800;
		//digitalWrite(_cs, LOW);
		delayMicroseconds(2*LTC_ACLK);

		//Uebertrage und lese 12Bit. Beginne mit dem höchsten Bit
		for (i = 12; i >= 1; i--) {

			//SCLK auf logisch 0 setzen
			digitalWrite(_sclk, LOW);
			//Uebertrage jeweiliges Steuerbit
			//DIN setzen
			digitalWrite(_din, ((kanal & bitwert)>0));
			delayMicroseconds(1);

			//SCLK auf logisch 1 setzen
			digitalWrite(_sclk, HIGH);
			//Wenn DOUT = 1 dann addiere bitwert zu messwert
			if (digitalRead(_dout))
				messwert += bitwert;
			//Halbiere den Bitwert, damit er den Wert des nächsten "i" Durchgangs hat
			bitwert = bitwert / 2;
			delayMicroseconds(1);
		} //Ende for (i=...)
		//digitalWrite(_cs, HIGH); //Conversion starts
		digitalWrite(_sclk, LOW);
		digitalWrite(_din, LOW);
		delayMicroseconds(LTC_CTIME*LTC_ACLK);

		if(ch>0){
			if(j>0) res[ch-1]= messwert;
			else res[ch-1]= messwert;

		}
	}
	}
	if(rep){
	   for(i=0;i<8;i++){
		   res[i]>>rep;
	   }
	}
	digitalWrite(_cs, HIGH);
}

int LTC1290::read(uint8_t ch,uint8_t rep) {
	digitalWrite(_cs, LOW);
	//Variablen deklarieren
	uint8_t i, j;
	if(rep>8) rep=8;

	int messwert, bitwert, kanal;
	long sum=0;
	//Kanal in Steuerbit wandeln
	// xXE: E = 1110 : SingleEnded - MSBF - 10==12-bits Word length
	switch (ch) {
	case 0:
		kanal = 0x8EF;
		break;
	case 1:
		kanal = 0xCEF;
		break;
	case 2:
		kanal = 0x9EF;
		break;
	case 3:
		kanal = 0xDEF;
		break;
	case 4:
		kanal = 0xAEF;
		break;
	case 5:
		kanal = 0xEEF;
		break;
	case 6:
		kanal = 0xBEF;
		break;
	case 7:
		kanal = 0xFEF;
		break;
	}
	//Insgesamt  (1<<rep) + 1 mal LTC1290 auslesen.
	//beim ersten mal Steuerbit übertragen und Messwert verwerfen
	//beim 2. mal Messwert endgültig einlesen
	for (j = 0; j <= (1<<rep); j++) {
		//Messwert und Bitwert auf Bit11 rücksetzen
		messwert = 0x00;
		bitwert = 0x800;
		//digitalWrite(_cs, LOW);
		delayMicroseconds(2*LTC_ACLK);

		//Uebertrage und lese 12Bit. Beginne mit dem höchsten Bit
		for (i = 12; i >= 1; i--) {

			//SCLK auf logisch 0 setzen
			digitalWrite(_sclk, LOW);
			//Uebertrage jeweiliges Steuerbit
			//DIN setzen
			digitalWrite(_din, ((kanal & bitwert)>0));
			delayMicroseconds(1);

			//SCLK auf logisch 1 setzen
			digitalWrite(_sclk, HIGH);
			//Wenn DOUT = 1 dann addiere bitwert zu messwert
			if (digitalRead(_dout))
				messwert += bitwert;
			//Halbiere den Bitwert, damit er den Wert des nächsten "i" Durchgangs hat
			bitwert = bitwert / 2;
			delayMicroseconds(1);
		} //Ende for (i=...)
		//digitalWrite(_cs, HIGH); //Conversion starts
		digitalWrite(_sclk, LOW);
		digitalWrite(_din, LOW);
		delayMicroseconds(LTC_CTIME*LTC_ACLK);

		if(j>0) sum+= messwert;
	} //Ende (j=0...
	sum>>=rep;
	digitalWrite(_cs, HIGH);

	return ((int) sum);
}

