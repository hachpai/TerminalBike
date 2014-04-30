
#include <Nrf2401.h>
//#include <rfid.h>
// digit 2 <-> 9
// 5V <-> 
// GND <->
#include <SoftwareSerial.h>

// serialReadLine ->
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
// <- fin SerialReadLine

// rFIDRead ->
boolean rFIDRead(SoftwareSerial *RFIDSerial, byte *rFIDCode) {
  byte i = 0;
  byte readByte = 0;
  byte tempByte = 0;
  byte checksum = 0;
  byte bytesread = 0;  
  if(RFIDSerial->available() > 0) {
    if((readByte = RFIDSerial->read()) == 2) {                  // check for header 
      bytesread = 0; 
      while (bytesread < 12) {                        // read 10 digit code + 2 digit checksum
        if( RFIDSerial->available() > 0) { 
          readByte = RFIDSerial->read();
          if((readByte == 0x0D)||(readByte == 0x0A)||(readByte == 0x03)||(readByte == 0x02)) { // if header or stop bytes before the 10 digit reading 
            return false;                                  // stop reading
          }

          // Do Ascii/Hex conversion:
          if ((readByte >= '0') && (readByte <= '9')) {
            readByte = readByte - '0';
          } else if ((readByte >= 'A') && (readByte <= 'F')) {
            readByte = 10 + readByte - 'A';
          }

          // Every two hex-digits, add byte to code:
          if (bytesread & 1 == 1) {
            // make some space for this hex-digit by
            // shifting the previous hex-digit with 4 bits to the left:
            rFIDCode[bytesread >> 1] = (readByte | (tempByte << 4));

          if (bytesread >> 1 != 5) {                // If we're at the checksum byte,
              checksum ^= rFIDCode[bytesread >> 1];       // Calculate the checksum... (XOR)
            };
          } else {
            tempByte = readByte;                           // Store the first hex digit first...
          };

          bytesread++;                                // ready to read next digit
        } 
      }

      if (bytesread == 12) {              // if 12 digit read is complete
        if(rFIDCode[5] == checksum) {
          return true;
        }
      }
    }
  }
  return false;
}
// <- fin RFIDRead




Nrf2401 Radio;
volatile unsigned char borne_id = 0;
volatile unsigned char BIKE_ID = 200;
//byte client_rfid[6]={10,20,34,12,11,42};
byte client_rfid[6] = {0,0,0,0,0,0};

volatile unsigned char request_code=0;
volatile unsigned char data_request = 0;

// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only 
// modified by the interrupt handler - a function that we never directly call
unsigned char CHANNEL_BROADCAST = 0; 
unsigned char CHANNEL_COM = 100;

SoftwareSerial RFIDSerial = SoftwareSerial(8,9);


int state=0;
void setup(void)
{
	Serial.begin(9600);
	pinMode(13, OUTPUT);
	attachInterrupt(0, messageReceived, RISING);  // look for rising edges on digital pin 2
	Radio.dataRate = 0;
	Radio.localAddress = 0; // For receiving broadcasting, card remain on local address 0.
	Radio.rxMode(1);
	RFIDSerial.begin(9600);
}
                     
void loop(void)
{
	//Radio.channel= 111;

	Radio.rxMode(1);
	if(rFIDRead(&RFIDSerial, &client_rfid[0])) {
		Serial.println("RFID READ");
		state = 1;
		//WARNING: To flush the serial buffer at the end of complete transaction
	}
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
		
		

	}
}

void sendRFID(){
	Radio.txMode(6);
	Radio.data[0] = 42; //withdraw code
	for(int i =0;i<6;i++){
		Radio.data[i+1] = client_rfid[i]; //6 bytes of RFID
		Serial.print(client_rfid[i]);
		Serial.print(" ");
	}
	Radio.write();
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
