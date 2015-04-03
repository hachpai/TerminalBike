#include "printf.h"
#include "RFCore.h"
#include <Servo.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "Rfid.h"
#include "Adafruit_NeoPixel.h"

/************************/
/** PINs configuration **/
/************************/

/* Button */
const int BUTTON_PIN1 = 14; //A0
const int BUTTON_PIN2 = 15; //A1

/* LED */
const int LED_PIN = 5; // NeoPixel
/*
const int RED_PIN = 5;
const int GREEN_PIN = 6;
const int BLUE_PIN = 7;
*/

/* RF
black module config: http://www.seeedstudio.com/document/pics/Interface.jpg
1 - GND       |8 7 |
2 - VCC       |6 5 |
3 - CE        |4 3 |
4 - CSN       |2[1]|
5 - SCK
6 - MOSI
7 - MISO
8 - IRQ */

/* pour RF24 radio(9,10) as in RFCore (http://tmrh20.github.io/RF24/ (avec 7 -> 9 et 8 -> 10))
GND -> GND
VCC -> 3V3
CE  -> 9
CSN ->10
MOSI ->11
MISO ->12
SCK ->13
IRQ -> 2
*/

/* RFID */
const int RFID_ACTIVATE = 6; // OUTPUT 1 TO ACTIVATE THE LED

const int RFID_IN = 7; //final version will use pin 0 (serial)
const int RFID_OUT = 8;//final version will use pin 1 (without serial.begin, it makes them unusable)

/*others constants*/
const int RFID_TIMEOUT = 10000; //10 sec to put the rfid card
const int USER_CODE_TIMEOUT = 10000; //10 sec to type the code
const int USER_CODE_LENGTH = 4;
const int MAX_ATTEMPT_TO_SEND_DATA = 3; // number attempts to send data to the born/terminal
const int DELAY_ATTEMPT_TO_SEND_DATA = 300; // 0.3 sec to wait between 2 attemps of sending data to the born/terminal

bool terminal_in_range = false;

Rfid rfid(RFID_IN,RFID_OUT);
RFCore * rf_core;

byte client_rfid[6] = {34,35,36,37,38,39};

unsigned char user_code[6]= {0,0,0,0,0,0}; //All char array about to be send must be the size of one packet

byte user_code_byte = 0x0;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, LED_PIN, NEO_KHZ800);

void setup(void) {
	Serial.begin(57600);

	MCUCR = (1<<ISC01) | (1<<ISC00); //01 and 00 for triggering interrupts on change at low level
  
	printf_begin();

	pinMode(BUTTON_PIN1, INPUT);
	pinMode(BUTTON_PIN2, INPUT);

	pinMode(RFID_ACTIVATE, OUTPUT);

  
	int BIKE_ID = 8;//will read EEPROM after
	rf_core = new RFCore(BIKE_ID, false);
	pixels.begin();

	disableRFID();
}

void switchButtonsInterrupts(boolean on) { //TRUE for ON, FALSE for off
	/* Pin to interrupt map:
	* D0-D7 = PCINT 16-23 = PCIE2 = pcmsk2
	* D8-D13 = PCINT 0-5 = PCIE0 = pcmsk0
	* A0-A5 (D14-D19) = PCINT 8-13 = PCIE1 = pcmsk1*/
	//interrupt on analog PIN A0 y A1
	if(on) { //enable interrupts
		PCICR |= (1<<PCIE1);
		PCMSK1 |= (1<<PCINT8); //8 for A0 pin change mask get loaded a pin change interrupt on A0.
		PCMSK1 |= (1<<PCINT9); //9 for A1
	}
	else {//disable interrupts
		PCICR &= ~(1<<PCIE1);
		PCMSK1 &= ~(1<<PCINT8); //8 for A0 pin change mask get loaded a pin change interrupt on A0.
		PCMSK1 &= ~(1<<PCINT9); //9 for A1
	}
}

void switchOnLed(char* color) {
	int r = 0;
	int g = 0;
	int b = 0;

	//Red
	if(color == "red" or color == "yellow" or color == "orange" or color == "white") {
		r = 255;
	}
	//Green
	if (color == "green" or color == "yellow" or color == "white") {
		g = 255;
	} else if(color == "orange") {
		g = 128;
	}
	//Blue
	if (color == "blue" or color == "white") {
		b = 255;
	}

	pixels.setPixelColor(0, pixels.Color(r,g,b));
	pixels.show();
}

void switchOffLed() {
	pixels.setPixelColor(0, pixels.Color(0,0,0));
	pixels.show();
}

/**
 * Set the bike in sleep mod. The bike is awaked when a button is pressed.
 */
void sleepNow() {
	//http://donalmorrissey.blogspot.com.es/2010/04/putting-arduino-diecimila-to-sleep.html
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

/**
 * Try to read the RFID Data. There is a timeout given by RFID_TIMEOUT
 *
 * Return true if RFID code whare read (the data is stored in client_rfid)
 * return false otherwise
 */
boolean getRFID() {
	int actual_time= millis();
	int initial_time = actual_time;
	boolean received = false;
	while((actual_time-initial_time) < RFID_TIMEOUT && !received) {
		received = rfid.RFIDRead(&client_rfid[0]);
		actual_time = millis();
		delay(50);
	}
	disableRFID();
	return received;
}


void enableRFID() {
	digitalWrite(RFID_ACTIVATE, HIGH);
}

void disableRFID() {
	digitalWrite(RFID_ACTIVATE, LOW);
}

/**
 * Try to get the User Code. 
 * This function is customized by the following constants :
 * - USER_CODE_TIMEOUT : timeout (in milliseconds)
 * - USER_CODE_LENGTH : the lenth of the code
 *
 * Return true if the code is read and store the variable in the variable user_code
 * Return false otherwise
 */
boolean getUserCode(){
	int button_state1=LOW,button_state2=LOW,index_key=0;
	int actual_time= millis();
	int initial_time = actual_time;
	boolean buttons_released =false;
	user_code_byte = 0x0;

	while((actual_time-initial_time) < USER_CODE_TIMEOUT && index_key < USER_CODE_LENGTH){
		actual_time = millis();
		button_state1 = digitalRead(BUTTON_PIN1);
		button_state2 = digitalRead(BUTTON_PIN2);

		if(button_state1== LOW && button_state2 == LOW){ // if both buttons are released
			buttons_released = true;
			digitalWrite(BUTTON_PIN1,LOW);
		}
		//check if user has released the buttons and if only one is pressed
		else if(buttons_released && (button_state1== HIGH ^ button_state2 == HIGH))
		{
			// verify which button is pressed and if not both are pressed
			Serial.println("INSIDE!");
			if (button_state1 == HIGH && button_state2 == LOW) {
				switchOffLed();
				Serial.println("INSIDE 1!");
				user_code[index_key] = 1;
				index_key++;

				user_code_byte = user_code_byte | 0X0;
				if(index_key != USER_CODE_LENGTH) {
					user_code_byte = user_code_byte << 1;
				}

				delay(200); //to avoid contact bounce
				switchOnLed("blue");
			}
			else if(button_state2 == HIGH && button_state1 == LOW){
				switchOffLed();
				Serial.println("INSIDE 2!");
				user_code[index_key] = 2;
				index_key++;

				user_code_byte = user_code_byte | 0X1;
				if(index_key != USER_CODE_LENGTH) {
					user_code_byte = user_code_byte << 1;
				}

				delay(200); //to avoid contact bounce
				switchOnLed("blue");
			}
			buttons_released = false;
		}
	}
	return (index_key == USER_CODE_LENGTH);
}

boolean canWithdraw() {
	if(rf_core->handShake()) {
		printf("Success handshake!\n\r");
		printf("Sending data\n\r");

		int response = rf_core->withdrawPermission(user_code_byte, &client_rfid);

		printf("response : %i\n\r", response);

		rf_core->closeSession();

		return response == 1;
	}
	return false;
}

void loop() {
	terminal_in_range = rf_core->rangeTest();
	printf("Awake\n\r");

	//rf_core->debug();

	if(terminal_in_range) {
		printf("Terminal in range!\n\r");
		switchOnLed("white");

		enableRFID();
		delay(500);

		if (!getRFID()) {
			printf("No RFID given\n\r");
				switchOnLed("red");
		} else {
			printf("Get RFID : ");
			for(int i =0; i<6;i++){
				Serial.print(client_rfid[i],HEX);
			}
			printf("\n\r");
			switchOnLed("blue");

			if(!getUserCode()) {
				printf("No user code given\n\r");
			} else {
				printf("User code given : ");
				
				for(int i =0; i<USER_CODE_LENGTH;i++){
					Serial.print(user_code[i]);
				}
				printf("\n\r");
				switchOnLed("yellow");

				printf("%i\n\r", user_code_byte);

				if(canWithdraw()) {
					printf("Ok withdraw\n\r");
					switchOnLed("green");
					delay(3000);
				} else {
					printf("No withdraw\n\r");
					switchOnLed("red");
					delay(3000);
				}
			}
		}
	}

	switchOffLed();
	printf("Go to sleep\n\r");
	sleepNow();
}
