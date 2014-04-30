
#include <Nrf2401.h>

Nrf2401 Radio;
volatile unsigned char borne_id = 0;
volatile unsigned char BIKE_ID = 200;
byte client_rfid[6]={10,20,34,12,11,42};

volatile unsigned char request_code=0;
volatile unsigned char data_request = 0;

// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only 
// modified by the interrupt handler - a function that we never directly call
unsigned char CHANNEL_BROADCAST = 0; 
unsigned char CHANNEL_COM = 100;

void setup(void)
{
	Serial.begin(9600);
	pinMode(13, OUTPUT);
	attachInterrupt(0, messageReceived, RISING);  // look for rising edges on digital pin 2
	Radio.dataRate = 0;
	Radio.localAddress = 0; // For receiving broadcasting, card remain on local address 0.
	Radio.rxMode(1);
}
                     
void loop(void)
{
	//Radio.channel= 111;

	Radio.rxMode(1);
	if(borne_id==0)
	{
		changeChannel(CHANNEL_BROADCAST);
		Serial.println("LISTENING...");
	}
	else{
		Serial.print("BORNE FOUND! ID:");
		Serial.println(borne_id);
		Radio.txMode(1);
		Radio.data[0] = BIKE_ID;
		Radio.write(); // Send his bike ID
		changeChannel(CHANNEL_COM);
		delay(1000); //wait for second arduino
		Radio.txMode(6);
		Radio.data[0] = 42; //withdraw code
		for(int i =0;i<6;i++){
			Radio.data[i+1] = client_rfid[i]; //6 bytes of RFID
		}
		Radio.write();
		Radio.rxMode(2);
		
		Serial.println("code received:");
		Serial.println(data_request);
		borne_id = 0;
		Radio.localAddress = 0; //to receive broadcast packets
		Radio.remoteAddress = 0;
		Serial.println("END");
		delay(2000);
	}
}

void messageReceived(void)
{
	//Serial.println("RECEIVED"); 
	Radio.read();
	if (Radio.channel == CHANNEL_BROADCAST)
	{
	//digitalWrite(13, HIGH);
	borne_id = Radio.data[0];
	Radio.remoteAddress = borne_id;
	Radio.localAddress = BIKE_ID;
	//Serial.println(borne_id);
	Radio.txMode(1);
	}
	else if (Radio.channel == CHANNEL_COM){
		request_code = Radio.data[0];
		data_request = Radio.data[1];
	}
}

void changeChannel(int new_channel){
	if(Radio.channel != new_channel){
		Radio.channel = new_channel;
		delay(250);
	}
}
