
#include <LiquidCrystal.h>
#include "RFCore.h"

RFCore * rf_core;

//byte client_rfid[6]={10,20,34,12,11,42};
unsigned char data[6] = {0,0,0,0,0,0};

// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only
// modified by the interrupt handler - a function that we never directly call


unsigned char BORNE_ID=42;

byte WITHDRAW_CODE=10;
byte RETURN_CODE=20;

int led =13;
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

byte status = 0;

void setup(void)
{
	Serial.begin(9600);
	pinMode(led, OUTPUT);
	lcd.begin(16, 2);
	rf_core = new RFCore(BORNE_ID, true);
}

byte locker_code[6] = {1,0,0,1,1,0};

void loop(void){
	while(!rf_core->handShake()){
		lcd.home();
		lcd.print("BROADCASTING...");
	}
	digitalWrite(led, HIGH);
	String result = "ID received:";
	result = result + rf_core->getRemoteID();
	displayInfos("BIKE FOUND!",result);
	//delay(1000);//wait for data

	boolean data_available = rf_core->getNextPacket(&data[0]); //get the operation code
	if(data_available){
		result = byteArrayToString(&data[0],6);
		displayInfos("Operation Code:",result);
		status = data[0]; //get the operation code from the bike
	}
	else{
		displayInfos("Data status:","not received.");
		status = 0;
	}

	switch(status){
		case WITHDRAW_CODE:
		withdrawBikeOperation();
		break;
		case RETURN_CODE:
		break;
		case 0:
		break;
	}
	rf_core->toDebug(); //DO THIS AT THE END!! strange effects otherwise
	delay(10000);
}

void withdrawBikeOperation(){
	data_available = rf_core->getNextPacket(&data[0]); //get the RFID
	if(data_available){
		result = byteArrayToString(&data[0],6);
		displayInfos("RFID Received:",result);
	}
	else{
		displayInfos("Data status:","not received.");
	}
	serialPrintLine(byteArrayToString(&client_rfid[0],5));
	String s = "";
	for(int i = 0;i<6;i++){
		s = s+client_rfid[i];
		if(i!=5){s = s + '-';}
	}
	lcd.setCursor(0, 0);
	lcd.print(s);
	delay(100); //wait for second arduino

	rf_core->sendPacket(locker_code);
}

void displayInfos(String l1, String l2){
	lcd.clear();
	lcd.home();
	lcd.print(l1);
	lcd.setCursor(0,1);
	lcd.print(l2);
}

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
