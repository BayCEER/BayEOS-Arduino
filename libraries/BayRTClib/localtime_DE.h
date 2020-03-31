/*
 * Header file for transformation of UTC-Time to localtime with daylight savings, holidays...
 *
 *
 */

#ifndef LOCALTIME_H
#define LOCALTIME_H
#include <BayEOSBuffer.h> /* for Datetime */

struct HOLIDAY {
	uint8_t day;
	uint8_t month;
};

//Returns true for all free days incl. Saturday and Sunday
bool isHoliday(DateTime& date){
	//Saturday or Sunday
	if(date.dayOfWeek()==0 || date.dayOfWeek()==6) return true;

	//Fixed day holidays
	HOLIDAY holidays[]={{1,1},{6,1},{3,10},{1,11},{25,12},{26,12}};
	for(uint8_t i=0;i<sizeof(holidays)/2;i++){
		if(holidays[i].day==date.day() && holidays[i].month==date.month()) return true;
	}

	//Variable date holidays (easter)
    //day shifts to easter sunday for Karfreitag, Ostermontag, Himmelfahrt, Pfingstmontag, Fronleichnam
    int8_t easter_shifts[]={-2,1,39,50,60};
	//calculate easter sunday
    int a = date.year() % 19;
    int b = date.year() /100;
    int c = date.year() %100;
    int d = b / 4;
    int e = b % 4;
    int g = (8 * b + 13) / 25;
    int h = (19 * a + b - d - g + 15) % 30;
    int j = c / 4;
    int k = c % 4;
    int m = (a + 11 * h) / 319;
    int r = (2 * e + 2 * j - k - h + m + 32) % 7;
    int n = (h - m + r + 90) / 25; //month
    int p = (h - m + r + n + 19) % 32; //day
    DateTime easter_sunday(date.year(),n,p);
    DateTime easter_holiday;
    for(uint8_t i=0;i<sizeof(easter_shifts);i++){
    	easter_holiday=DateTime(easter_sunday.get()+86400L*easter_shifts[i]);
    	if(date.day()==easter_holiday.day() && date.month()==easter_holiday.month()) return true;
    }
    return false;
}


//Returns true when there is daylight saving for the date
bool isDaylightSaving(DateTime& d){
	DateTime temp;
	if(d.month()<3) return false;
	if(d.month()>10) return false;
	if(d.month()>3 && d.month()<10) return true;
	if(d.month()==3){
		temp=DateTime(d.year(),3,31);
		while(temp.dayOfWeek()){ //not Sunday
			temp=DateTime(temp.get()-86400L);
		}
		if(d.day()<temp.day()) return false;
		return true;
	}
	if(d.month()==10){
		temp=DateTime(d.year(),10,31);
		while(temp.dayOfWeek()){ //not Sunday
			temp=DateTime(temp.get()-86400L);
		}
		if(d.day()<temp.day()) return true;
		return false;
	}
}

//get the time shift for the date
long getShift(DateTime& d){
	if(isDaylightSaving(d)) return 7200; //Two hours ahead from UTC
	else return 3600; //One hour ahead from UTC
}

#endif
