/* 
thermostat firmware
(c) 2017 Robert Fonó

available serial commands:

t:%d%d - set mock temperature
example: t:18

c:%d%d%d%d - set clock in format HHMM
example: c:1608

*/


#include "LiquidCrystal.h"
#include <inttypes.h>
#include <Wire.h>
#include "RTClib.h"

int CTRL = 8;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

RTC_DS1307 RTC;


int temp = 19;
int daytarget = 22;
int nighttarget = 18;

int target = daytarget;

int daylimit_hour = 7;
int daylimit_minute = 0;
int nightlimit_hour = 21;
int nightlimit_minute = 0;
 
char *state = "stdb";
char *mode = "day";

//utility variables

int loopIter = 0;

void lcdTime() {
	DateTime now = RTC.now();

	lcd.setCursor(0, 0);
	
	if(now.hour()<10) {
		lcd.print('0');
		lcd.print(now.hour());
	} else {
		lcd.print(now.hour());
	}

	lcd.print(":");

	if(now.minute()<10) {
		lcd.print('0');
		lcd.print(now.minute());
	} else {
		lcd.print(now.minute());
	}
}
void dispView() {

	lcd.clear();
	
	lcdTime();

	lcd.setCursor(7,0);

	char tempstr[8]; 
	sprintf(tempstr, "%dC", temp);
	lcd.print(tempstr);

	lcd.setCursor(11, 0);

	char tgtstr[7];
	sprintf(tgtstr, "->%dC", target);
	lcd.print(tgtstr);

	lcd.setCursor(0, 1);

	char ststr[8];
	sprintf(ststr, "ST:%s", state);
	lcd.print(ststr);

	lcd.setCursor(8,1);

	char mostr[8];
	sprintf(mostr, "MO:%s", mode);
	lcd.print(mostr);
}

void serialTime() {
	DateTime now = RTC.now(); 
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print('/');
	Serial.print(now.year(), DEC);
	Serial.print(' ');
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();    
}

void serialProcess(char* indata) {
	if (strcmp(indata,"night") == 0){
		mode = "night";
		target = nighttarget;
		dispView();
	} 
	else if (strcmp(indata,"day") == 0){
		mode = "day";
		target = daytarget;
		dispView();
	}
	else if (indata[0] == 't' && indata[1] == ':') {
		char convtemp[2] = {indata[2], indata[3]};
		temp = atoi(convtemp);
		dispView();
	}
	else if (strcmp(indata,"time") == 0){
		serialTime();
		delay(100);
	}
	else if (indata[0] == 'c' && indata[1] == ':') {
		char newclock[8] = {	indata[2], indata[3], ':', 
					indata[4], indata[5], ':', '0', '0'};
		
		RTC.adjust(DateTime(__DATE__, newclock));
		lcdTime();
	}

}

//decide if it's day or night
//returns name of mode ("day" or "night") 
char *pickMode(int hour, int minute) {

	if (	(hour > daylimit_hour || 
		(hour == daylimit_hour && minute >= daylimit_minute)) &&
		(hour < nightlimit_hour || 
		(hour == nightlimit_hour && minute < nightlimit_minute)) 
	) {
		return "day";
	} else {
		return "night";
	}

}

void setTarget() {
	if (strcmp(mode,"day") == 0) {
		target = daytarget;
	} else if (strcmp(mode, "night") == 0) {
		target = nighttarget;
	}
}


void setup() {
	lcd.begin(16, 2);
	Serial.begin(9600);
	Wire.begin();
	RTC.begin();

	if (! RTC.isrunning()) {
		 Serial.println("RTC is NOT running!");
		// This will reflect the time that your sketch was compiled
		RTC.adjust(DateTime(__DATE__, __TIME__));
	}

	
	pinMode(CTRL, OUTPUT);
	digitalWrite(CTRL, LOW);
	dispView();
}

void loop() {

	//refresh the clock if it hasn't been refreshed in a while
	//fires roughly every 5 seconds
	loopIter++;
	if(loopIter >= 100) {
		lcdTime();
		loopIter = 0;
	}

	char indata[7] = {'\0'};
	char inchar = -1;
	int index = 0;

	while (Serial.available() > 0) {
		if (index < 6) {
			inchar = Serial.read();
			indata[index] = inchar;
			index++;
			indata[index] = '\0';
		} 
		else {
			Serial.read();		
		}
	}
	delay(50);
	
	serialProcess(indata);
	

	//run pickMode, redraw screen if mode changed

	DateTime now = RTC.now();

	char *newMode = pickMode(now.hour(), now.minute());
	
	if (!strcmp(newMode, mode) == 0) {
		mode = newMode;
		setTarget();
		dispView();	
	}

	if (temp < target) {
		digitalWrite(CTRL, HIGH);
		if (strcmp(state,"heat") != 0) {
			state = "heat";
			dispView();
		}
	} else {
		digitalWrite(CTRL, LOW);
		if (strcmp(state,"stdb") != 0) {
			state = "stdb";
			dispView();
		}
	}


}
