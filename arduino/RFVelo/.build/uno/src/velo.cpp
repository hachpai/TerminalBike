#include <Arduino.h>
#include "Rfid.h"
#include "RFCore.h"
#include <avr/sleep.h>
#include <avr/power.h>
void setup(void);
void loop(void);
void switchButtonsInterrupts(boolean on);
void sleepNow();
boolean withdrawBike();
void rgbLed(byte color);
boolean getRFID();
boolean getUserCode();
#line 1 "src/velo.ino"
//#include "Rfid.h"
//#include "RFCore.h"
//#include <avr/sleep.h>
//#include <avr/power.h>
/*PINs configuration*/

//const int MOTOR = 7; //final version will use this pin for the unlocker motor
//const int BUZZER = 8;//final version will use this pin for the sound buzzer

const int SERIAL_IN = 7; //final version will use pin 0
const int SERIAL_OUT = 8;//final version will use pin 1 (without serial.begin, it makes them unusable)

const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

const int BUTTON_PIN1 = 14;
const int BUTTON_PIN2 = 15;

/*others constants*/

const int USER_CODE_TIMEOUT = 10000; //10 sec to type the code
const int USER_CODE_LENGTH = 4;
const int RFID_TIMEOUT = 10000; //10 sec to put the rfid card

const byte LED_OFF=0;
const byte LED_RED=1;
const byte LED_GREEN=2;
const byte LED_YELLOW=3;

Rfid rfid(SERIAL_IN,SERIAL_OUT);
RFCore * rf_core;

unsigned int BIKE_ID = 200;
//byte client_rfid[6]={10,20,34,12,11,42};
byte client_rfid[6] = {34,35,36,37,38,39};
unsigned char data[6]= {0,0,0,0,0,0};
unsigned char user_code[4]= {0,0,0,0};

volatile unsigned char request_code=0;
volatile unsigned char data_request = 0;

const byte LOCKED_TERMINAL = 1;
const byte LOCKED_AWAY= 2;
const byte UNLOCKED = 3;
byte state=LOCKED_TERMINAL;

void setup(void)
{
	Serial.begin(9600);
	MCUCR = (1<<ISC01) | (1<<ISC00); //01 and 00 for triggering interrupts on change at low level
	/*to test!*/
	//PRR = bit(PRTIM1);                           // only keep timer 0 going
	//ADCSRA &= ~ bit(ADEN); bitSet(PRR, PRADC);   // Disable the ADC to save power
	pinMode(RED_PIN, OUTPUT);
	pinMode(GREEN_PIN, OUTPUT);
	pinMode(BLUE_PIN, OUTPUT);
	pinMode(BUTTON_PIN1, INPUT);
	pinMode(BUTTON_PIN2, INPUT);
	pinMode(13,OUTPUT);
	digitalWrite(13,LOW);
	//digitalWrite(BUTTON_PIN1,HIGH); //Enable pullup resistor on Analog Pin
	//digitalWrite(BUTTON_PIN2,HIGH);//Enable pullup resistor on Analog Pin
	rf_core = new RFCore(BIKE_ID, false);
	interrupts();
}

void loop(void)
{
	switch(state){

		case LOCKED_TERMINAL:
		if(withdrawBike()){
			state = UNLOCKED;
			delay(1000); //to let the green LED works one second, notice the success
			rgbLed(LED_OFF);
		}
		break;
		case UNLOCKED:
		break;
	}
	rgbLed(LED_OFF);
	sleepNow();
}

ISR(PCINT1_vect) { //WARNING: comment the ISR(PCINT1_vect) definition in SofwareSerial.cpp
	switchButtonsInterrupts(false); //disable interrupt on buttons, arduino is awake, that avoid the interrupt to fire after wake.
}

void switchButtonsInterrupts(boolean on){ //TRUE for ON, FALSE for off
	/* Pin to interrupt map:
	* D0-D7 = PCINT 16-23 = PCIE2 = pcmsk2
	* D8-D13 = PCINT 0-5 = PCIE0 = pcmsk0
	* A0-A5 (D14-D19) = PCINT 8-13 = PCIE1 = pcmsk1*/
	//interrupt on analog PIN A0 y A1
	if(on){ //enable interrupts
		PCICR |= (1<<PCIE1);
		PCMSK1 |= (1<<PCINT8); //8 for A0 pin change mask get loaded a pin change interrupt on A0.
		PCMSK1 |= (1<<PCINT9); //9 for A1
	}
	else{//disable interrupts
		PCICR &= ~(1<<PCIE1);
		PCMSK1 &= ~(1<<PCINT8); //8 for A0 pin change mask get loaded a pin change interrupt on A0.
		PCMSK1 &= ~(1<<PCINT9); //9 for A1
	}
}
//http://donalmorrissey.blogspot.com.es/2010/04/putting-arduino-diecimila-to-sleep.html
void sleepNow()
{
	switchButtonsInterrupts(true);
	delay(100);
	// Choose our preferred sleep mode:
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	//
	//interrupts();
	//
	// Set sleep enable (SE) bit:
	sleep_enable();
	//
	// Put the device to sleep:
	digitalWrite(13,LOW);   // turn LED off to indicate sleep
	sleep_mode();
	//
	// Upon waking up, sketch continues from this point.

	sleep_disable();
	digitalWrite(13,HIGH);   // turn LED on to indicate awake
}

boolean withdrawBike(){
	rgbLed(LED_RED);
	Serial.println("RFID READ");
	if(!getRFID()) return false; //if we don't get the rfid code, failure

	for(int i =0; i<6;i++){
		Serial.print(client_rfid[i],HEX);
	}
	rgbLed(LED_YELLOW);
	if(!getUserCode()) return false;//if we don't get the user code, failure
	Serial.println(' ');
	Serial.println("Listening...");
	while(!rf_core->handShake()){
		delay(50); //wait for the handshake
	}
	String result = "ID received:";
	result = result + rf_core->getRemoteID();
	Serial.println(result);
	data[0]=20;
	rf_core->sendPacket(data); //send operation code
	rf_core->sendPacket(client_rfid); //send RFID customer

	delay(1000);//wait data
	rf_core->getNextPacket(data);
	rf_core->toDebug();
	if(data[0]==1){
		rgbLed(LED_GREEN);
		//move servomotor;
		return true;
	}
	return false;
}

void rgbLed(byte color)
{
	switch (color) {
		case LED_OFF:
		// Off (all LEDs off):
		digitalWrite(RED_PIN, LOW);
		digitalWrite(GREEN_PIN, LOW);
		digitalWrite(BLUE_PIN, LOW);
		break;
		case LED_RED:
		// Red (turn just the red LED on):
		digitalWrite(RED_PIN, HIGH);
		digitalWrite(GREEN_PIN, LOW);
		digitalWrite(BLUE_PIN, LOW);
		break;
		case LED_GREEN:
		// Green (turn just the green LED on):
		digitalWrite(RED_PIN, LOW);
		digitalWrite(GREEN_PIN, HIGH);
		digitalWrite(BLUE_PIN, LOW);
		case LED_YELLOW:
		// Yellow (turn red and green on):
		digitalWrite(RED_PIN, HIGH);
		digitalWrite(GREEN_PIN, HIGH);
		digitalWrite(BLUE_PIN, LOW);
		break;
		default:
		// Off (all LEDs off):
		digitalWrite(RED_PIN, LOW);
		digitalWrite(GREEN_PIN, LOW);
		digitalWrite(BLUE_PIN, LOW);
	}
}

boolean getRFID(){
	int actual_time= millis();
	int initial_time = actual_time;
	boolean received = false;
	while((actual_time-initial_time) < RFID_TIMEOUT && !received) {
		received = rfid.RFIDRead(&client_rfid[0]);
		actual_time = millis();
		delay(50);
	}
	return received;
}

boolean getUserCode(){
	int button_state1=LOW,button_state2=LOW,index_key=0;
	int actual_time= millis();
	int initial_time = actual_time;
	boolean buttons_released =false;
	while((actual_time-initial_time) < USER_CODE_TIMEOUT && index_key < USER_CODE_LENGTH){
		actual_time = millis();
		button_state1 = digitalRead(BUTTON_PIN1);
		button_state2 = digitalRead(BUTTON_PIN2);

		if(button_state1== LOW && button_state2 == LOW){ // if both buttons are released
			buttons_released = true;
		}
		//check if user has released the buttons and if only one is pressed
		else if(buttons_released && (button_state1== LOW || button_state2 == LOW))
		{
			// verify which button is pressed and if not both are pressed
			if (button_state1 == HIGH && button_state2 == LOW) {
				user_code[index_key] = 1;
			}
			else if(button_state2 == HIGH && button_state1 == LOW){
				user_code[index_key] = 2;
			}
			buttons_released = false;
			index_key++;
		}
	}
	return (index_key == USER_CODE_LENGTH);
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
}
//Radio.channel= 111;

/*	Radio.rxMode(1);

//WARNING: To flush the serial buffer at the end of complete transaction
if(rf_core->getNextPacket(&data[0])){
Serial.println("code received:");
for(int i =0; i<6;i++){
Serial.print(data[i]);
}
Serial.print('\n');
}
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
