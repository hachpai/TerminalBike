#include "Rfid.h"
#include "RFCore.h"

Rfid rfid(8,9);

RFCore * rf_core;

unsigned int BIKE_ID = 200;
//byte client_rfid[6]={10,20,34,12,11,42};
byte client_rfid[6] = {0,0,0,0,0,0};
unsigned char data[6]= {0,0,0,0,0,0};

volatile unsigned char request_code=0;
volatile unsigned char data_request = 0;

int state=0;

void setup(void)
{
	Serial.begin(9600);
	pinMode(13, OUTPUT);
  rf_core = new RFCore(BIKE_ID, false);
}

void loop(void)
{
	if(rfid.RFIDRead(&client_rfid[0])) {
		Serial.println("RFID READ");
		state = 1;
		for(int i =0; i<6;i++){
			Serial.print(client_rfid[i],HEX);
		}
		Serial.print('\n');
		//delay(1000);

	}
	Serial.println("NO RFID");
	while(!rf_core->handShake()){
		Serial.println("Listening...");
	}
	String result = "ID received:";
	result = result + rf_core->getRemoteID();
	Serial.println(result);
	data[0]=20;
	rf_core->sendPacket(data);
	Serial.print("bou");
	//Radio.channel= 111;

/*	Radio.rxMode(1);

		//WARNING: To flush the serial buffer at the end of complete transaction
	}
	if(borne_id==0)
	{

	}
	else{
		Serial.print("BORNE FOUND! ID:");
		Serial.println(borne_id);
		Radio.txMode(1);
		Radio.data[0] = BIKE_ID;
		Radio.write(); // Send his bike ID
		changeChannel(CHANNEL_COM);
		delay(1000); //wait for second arduino
		sendRFID();
		Radio.rxMode(2);
		while(request_code != 45); // RC of 43 for user code reception
		Serial.println("user code received:");
		Serial.println(data_request);
		borne_id = 0;
		Radio.localAddress = 0; //to receive broadcast packets
		Radio.remoteAddress = 0;
		state=0;
		for(int i =0;i<6;i++){
			client_rfid[i] =0; //6 bytes of RFID
		}
		Serial.println("END");
		delay(2000);



	}*/
}

/**void sendRFID(){
	Radio.txMode(6);
	Radio.data[0] = 42; //withdraw code
	for(int i =0;i<6;i++){
		Radio.data[i+1] = client_rfid[i]; //6 bytes of RFID
		Serial.print(client_rfid[i]);
		Serial.print(" ");
	}
	Radio.write();
}**/
