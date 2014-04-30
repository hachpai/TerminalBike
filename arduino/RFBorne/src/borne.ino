
#include <Nrf2401.h>

Nrf2401 Radio;
volatile unsigned char request_code = 0;
volatile unsigned char bike_id = 0;
byte client_rfid[6];
// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only 
// modified by the interrupt handler - a function that we never directly call


unsigned char BORNE_ID=42;

unsigned char CHANNEL_BROADCAST = 0; 
unsigned char CHANNEL_COM = 100;


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
void setup(void)
{
	Serial.begin(9600);
	pinMode(led, OUTPUT);
	attachInterrupt(0, messageReceived, RISING);  // look for rising edges on digital pin 2
	Radio.dataRate = 0;
	Radio.localAddress = BORNE_ID;
	Radio.txMode(1);
}

void loop(void)
{
	changeChannel(CHANNEL_BROADCAST);
	if(bike_id==0)
	{
		//Serial.println("BROADCASTING...");
		Radio.txMode(1);
		Radio.write(BORNE_ID);
		Radio.rxMode(1);
		delay(500);
		
	}
	else{
		changeChannel(CHANNEL_COM);
		//Serial.print("bike found ID:");
		//Serial.println(bike_id);
		Radio.rxMode(6);
		while(request_code != 42); //To test, the delay(500); doesnt certify the reception of datas
		/*for(int i = 0;i<6;i++){
		//Serial.print(client_rfid[i]);
		//Serial.print(" ");
		}*/
		serialPrintLine(byteArrayToString(&client_rfid[0],5));
		
		delay(100); //wait for second arduino
		String userCode = "";
		while(!serialReadLine(userCode));
		//serialPrintLine(userCode);
		if (userCode == "0111") {
			digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
			delay(1000);               // wait for a second
			digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
			delay(1000); 
		}
		Radio.txMode(2);
		Radio.data[0]= 45;
		Radio.data[1]= userCode[0]; //get the code from DB
		Radio.write();
		//Serial.print("END!!");
		bike_id = 0;
		Radio.remoteAddress = 0;

	}
}

	void messageReceived(void)
	{
		if(Radio.available()){
			//Serial.println("DATA RECEIVED!");
			Radio.read();
			if(Radio.channel == CHANNEL_BROADCAST){
				bike_id = Radio.data[0];
				Radio.remoteAddress = bike_id;
			}
			else if(Radio.channel == CHANNEL_COM){
				request_code = Radio.data[0];
				for(int i = 0;i<6;i++){
					client_rfid[i] = Radio.data[i+1];
				}
			}
			Radio.txMode();

		}
	}
	void changeChannel(int new_channel){
		if(Radio.channel != new_channel){
			Radio.channel = new_channel;
			delay(250);
		}
	}
