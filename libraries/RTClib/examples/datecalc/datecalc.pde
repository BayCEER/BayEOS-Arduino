//>>> The latest version of this code can be found at https://github.com/jcw/ !!

// Simple date conversions and calculations
// 2010-02-04 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// $Id: datecalc.pde 7763 2011-12-11 01:28:16Z jcw $

#include <Wire.h>
#include <BayEOSBuffer.h>
#include <RTClib.h>

void showDate(const char* txt, const DateTime& dt) {
    Serial.print(txt);
    Serial.print(' ');
    Serial.print(dt.year(), DEC);
    Serial.print('/');
    Serial.print(dt.month(), DEC);
    Serial.print('/');
    Serial.print(dt.day(), DEC);
    Serial.print(' ');
    Serial.print(dt.hour(), DEC);
    Serial.print(':');
    Serial.print(dt.minute(), DEC);
    Serial.print(':');
    Serial.print(dt.second(), DEC);
    
    Serial.print(" = ");
    Serial.print(dt.get());
    Serial.print("s / ");
    Serial.print(dt.get() / 86400L);
    Serial.print("d since 2000");
    
    Serial.println();
}

void setup () {
    Serial.begin(57600);
    
    DateTime dt0 (0, 1, 1);
    showDate("dt0", dt0);

    DateTime dt1 (1, 1, 1);
    showDate("dt1", dt1);

    DateTime dt2 (2009, 1, 1, 0, 0, 0);
    showDate("dt2", dt2);

    DateTime dt3 (2009, 1, 2, 0, 0, 0);
    showDate("dt3", dt3);

    DateTime dt4 (2009, 1, 27, 0, 0, 0);
    showDate("dt4", dt4);

    DateTime dt5 (2009, 2, 27, 0, 0, 0);
    showDate("dt5", dt5);

    DateTime dt6 (2009, 12, 27, 0, 0, 0);
    showDate("dt6", dt6);

    DateTime dt7 = dt6.get() + 3600; // one hour later
    showDate("dt7", dt7);

    DateTime dt8 = dt6.get() + 86400L; // one day later
    showDate("dt8", dt8);

    DateTime dt9 = dt6.get() + 7 * 86400L; // one week later
    showDate("dt9", dt9);
}

void loop () {
}
