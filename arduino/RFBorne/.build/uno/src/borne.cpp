#include <Arduino.h>
#include <LiquidCrystal.h>
#include "RFCore.h"
void setup(void);
void loop(void);
void withdrawBikeOperation();
boolean serialReadLine(String &dest);
void sendDataToDB(unsigned char *byteArray, int packet_size);
void displayInfos(String l1, String l2);
String byteArrayToString(byte *byteArray, int byteArraySize);
#line 1 "src/borne.ino"

//#include <LiquidCrystal.h>
//#include "RFCore.h"

RFCore * rf_core;

byte client_rfid[6]={12,13,14,15,16,17};
unsigned char data[6] = {0,0,0,0,0,0};

byte accepted_code[6] = {200,200,200,200,200,200};
byte refused_code[6] = {100,100,100,100,100,100};
// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only
// modified by the interrupt handler - a function that we never directly call

unsigned char BORNE_ID=42;

const byte WITHDRAW_CODE=10;
const byte RETURN_CODE=20;

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

//send formated datas to nodeJS/mongoDB

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
		default:
		break;
	}

	delay(10000);
}

void withdrawBikeOperation(){
	bool data_available = rf_core->getNextPacket(&data[0]); //get the RFID
	if(data_available){
		String result = byteArrayToString(&data[0],6);
		for(int i = 0;i<6;i++){
			client_rfid[i]=data[i];
		}
		displayInfos("RFID Received:",result);
	}
	else{
		displayInfos("Data status:","not received.");
	}

	data_available = rf_core->getNextPacket(&data[0]); //get the RFID
	String user_code_received="";
	if(data_available){
		for(int i=0;i<4;i++){
			user_code_received += String(data[i],DEC);
		}
	}
	else{
		displayInfos("Data status:","not received.");
	}
	sendDataToDB(&client_rfid[0],5);//send 5 first bytes of RFID, 6th is checksum
	String user_code_in_db;
	delay(200); //wait for code
	while(!serialReadLine(user_code_in_db));//wait for response from DB
	Serial.print("code received:");
	Serial.println(user_code_received);
	Serial.print("code in DB:");
	Serial.println(user_code_in_db);
	if(user_code_received.compareTo(user_code_in_db)==0){
		Serial.println("code accepted.");
		rf_core->sendPacket(accepted_code);
	}
	else{
		Serial.println("code refused.");
		rf_core->sendPacket(refused_code);
	}
	rf_core->toDebug(); //DO THIS AT THE END!! strange effects otherwise


	//serialPrintLine(byteArrayToString(&client_rfid[0],5));
	//delay(1000000); //wait for second arduino
	rf_core->reset();
}


boolean serialReadLine(String &dest) {
	//Renvoie la taille lue... sur un tableau de char...
	String ret = "";
	char character;
	while (Serial.available()) {
		character = Serial.read();
		if(character == '\n') {
			dest = ret;
			return true;
		}
		else{
			ret+=character;
		}
	}
	return false;
}

void sendDataToDB(unsigned char *byteArray, int packet_size){

	String s= "[";

	for(int i = 0; i <packet_size;i++){
		if(byteArray[i]<10) s+="0"; // to avoid 0X to become X
		s+=String(byteArray[i],HEX);
	}
	s+="]";//end of transmission
	//s+='\0';//null for Serial sending
	Serial.println(s);
}

void displayInfos(String l1, String l2){
	lcd.clear();
	lcd.home();
	lcd.print(l1);
	lcd.setCursor(0,1);
	lcd.print(l2);
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
// <- fin byteArrayToString
