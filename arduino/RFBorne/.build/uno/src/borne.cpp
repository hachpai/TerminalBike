#include <Arduino.h>
#include <LiquidCrystal.h>
#include "RFCore.h"
void serialPrintLine(String s);
String byteArrayToString(byte *byteArray, int byteArraySize);
boolean serialReadLine(String &dest);
void setup(void);
void loop(void);
void displayInfos(String l1, String l2);
#line 1 "src/borne.ino"

//#include <LiquidCrystal.h>
//#include "RFCore.h"

RFCore * rf_core;

unsigned int BIKE_ID = 200;
//byte client_rfid[6]={10,20,34,12,11,42};
unsigned char data[6] = {0,0,0,0,0,0};

// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only
// modified by the interrupt handler - a function that we never directly call


unsigned char BORNE_ID=42;

byte WITHDRAW_CODE=10;
byte RETURN_CODE=20;

// serialPrintLine ->
void serialPrintLine(String s) {
	Serial.print(s);
	Serial.println();
}
// <- fin SerialPrintLine

// byteArrayToString ->
String byteArrayToString(byte *byteArray, int byteArraySize) {
	int i;
	String ret = "";
	for (i=0; i<byteArraySize; i++) {
		if(byteArray[i] < 16) ret += "0";
		ret += String(byteArray[i], HEX);
		if (i < 4) ret += " ";
	}
	return ret;
}
// <- fin byteArrayToString

boolean serialReadLine(String &dest) {
	//Renvoie la taille lue... sur un tableau de char...
	String ret = "";
	byte character;
	while (Serial.available()) {
		character = Serial.read();
		if(character == '\n') {
			dest = ret;
			return true;
		}
		else {
			ret.concat(character);
		}
	}
	return false;
}

int led =13;
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
void setup(void)
{
	Serial.begin(9600);
	pinMode(led, OUTPUT);
	lcd.begin(16, 2);
	rf_core = new RFCore(BORNE_ID, true);
}

void loop(void){
	while(!rf_core->handShake()){
		lcd.home();
		lcd.print("BROADCASTING...");
		Serial.println("counter:");
		Serial.println(rf_core->getCounter());
	}

	digitalWrite(led, HIGH);
	rf_core->printSerialBuffers();

	String result = "ID received:";
	result = result + rf_core->getRemoteID();
	displayInfos("BIKE FOUND!",result);
	Serial.println("counter:");
	Serial.println(rf_core->getCounter());
	delay(1000);//wait for data
	Serial.println("counter:");
	Serial.println(rf_core->getCounter());
	boolean data_received = rf_core->getNextPacket(&data[0]); //get the operation code
	Serial.println("counter:");
	Serial.println(rf_core->getCounter());
	rf_core->printSerialBuffers();
	if(data_received){
	result = byteArrayToString(&data[0],6);
	displayInfos("Operation Code:",result);
	Serial.println(result);
	}
	else displayInfos("Data status:","not received.");
	rf_core->printSerialBuffers();
	delay(20000);
	delay(1000);
	Serial.println("counter:");
	Serial.println(rf_core->getCounter());
	Serial.print("SECOND CALL");
	data_received = rf_core->getNextPacket(&data[0]); //get the RFID
	Serial.println("counter:");
	Serial.println(rf_core->getCounter());
	result = byteArrayToString(&data[0],6);

	displayInfos("RFID Received:",result);

}

void displayInfos(String l1, String l2){
	lcd.clear();
	lcd.home();
	lcd.print(l1);
	lcd.setCursor(0,1);
	lcd.print(l2);
}
