
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

void setup(void)
{
	Serial.begin(9600);
	pinMode(13, OUTPUT);
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
		Serial.println("BROADCASTING...");
		Radio.txMode(1);
		Radio.write(BORNE_ID);
		Radio.rxMode(1);
		delay(500);
		
	}
	else{
		changeChannel(CHANNEL_COM);
		Serial.print("bike found ID:");
		Serial.println(bike_id);
		Radio.rxMode(6);
		while(request_code != 42); //To test, the delay(500); doesnt certify the reception of datas
		for(int i = 0;i<6;i++){
			Serial.print(client_rfid[i]);
			Serial.print(" ");
		}
		//put DB CODE here
		Radio.txMode(2);
		Radio.data[0]= 45;
		Radio.data[1]= 01;
		Radio.write();
		Serial.print("END!!");
		bike_id = 0;
		Radio.remoteAddress = 0;
	}
}

void messageReceived(void)
{
	if(Radio.available()){
		Serial.println("DATA RECEIVED!");
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
