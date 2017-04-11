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

char *state = "stdb";
char *mode = "day";

void dispView() {

	DateTime now = RTC.now();

	lcd.clear();
	lcd.setCursor(0, 0);
	
	lcd.print(now.hour());
	lcd.print(":");
	lcd.print(now.minute());

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

	char indata[6] = {'\0'};
	char inchar = -1;
	int index = 0;

	while (Serial.available() > 0) {
		if (index < 5) {
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
