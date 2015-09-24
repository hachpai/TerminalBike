#include "printf.h"
#include "RFCore.h"
#include "Rfid.h"
#include <Servo.h>
#include "Adafruit_NeoPixel.h"
#include <avr/sleep.h>
#include <avr/power.h>

char test_mode='0';
/******************************/
/*     PINs CONFIGURATION     */
/******************************/
/* boutons */
const int BUTTON_PIN1 = 14; //A0
const int BUTTON_PIN2 = 15; //A1
/* Locker */
const int MOTOR_PIN_CONTROL = 6; //A4
const int MOTOR_ACTIVATE = 7; //A2;
/*RF*/
const int RF_PIN_CE=9;
const int RF_PIN_CSN=10;
/* RFID */
const int RFID_ACTIVATE = 3; // OUTPUT 1 TO ACTIVATE THE LED
const int RFID_DATA = 8;
/* LED */
const int LED_PIN = 5; // NeoPixel

//RF Variables

RFCore * rf_core;


//RFID
Rfid rfid(RFID_DATA);
byte rfid_data[6] = {0,0,0,0,0,0};
byte client_rfid[6] = {34,35,36,37,38,39};

//servo
Servo locker_motor;  // create servo object to control a servo


/*LED*/
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, LED_PIN, NEO_KHZ800);
const int COLOUR_OFF=0;
const int COLOUR_RED=1;
const int COLOUR_YELLOW=2;
const int COLOUR_ORANGE=3;
const int COLOUR_GREEN=4;
const int COLOUR_BLUE=5;
const int COLOUR_WHITE=6;

/*buttons*/
const int DEBOUNCE_DELAY =300;
byte user_code_byte = 0x0;

const int RFID_TIMEOUT = 5000; //10 sec to put the rfid card
const int USER_CODE_TIMEOUT = 5000; //10 sec to type the code
const int USER_CODE_LENGTH = 4;
unsigned char user_code[6]= {0,0,0,0,0,0}; //All char array about to be send must be the size of one packet
unsigned char correct_user_code[6]= {0,1,0,1,0,0}; //All char array about to be send must be the size of one packet

void setup(void) {
	Serial.begin(57600);

	MCUCR = (1<<ISC01) | (1<<ISC00); //01 and 00 for triggering interrupts on change at low level

	printf_begin();

	pinMode(BUTTON_PIN1, INPUT_PULLUP);
	pinMode(BUTTON_PIN2, INPUT_PULLUP);

	pinMode(RFID_ACTIVATE, OUTPUT);
	pinMode(MOTOR_ACTIVATE, OUTPUT);
	locker_motor.attach(MOTOR_PIN_CONTROL);  // attaches the servo on pin 9 to the servo object
	int BIKE_ID = 8;//will read EEPROM after
	rf_core = new RFCore(BIKE_ID, false);
	pixels.begin();
	interrupts();
	switchButtonsInterrupts(false);
	disableRFID();
}
int bike_state=0;
const int BIKE_IN_TERMINAL=0;
const int BIKE_AWAY=1;
const int BIKE_AWAY_LOCKED=3;

bool disfunction_signalized=false;
void loop() {
	readInitialButtonStates();
	printf("STATE:%d\n\r",bike_state);
	//sleepNow();
	switch(bike_state){
		case BIKE_IN_TERMINAL:
		if(disfunction_signalized){
			blinkLed(5,COLOUR_RED,200);
			blinkLed(2,COLOUR_GREEN,500);
			disfunction_signalized=false;
		}
		else{
			withdrawBike();
		}
		break;
		case BIKE_AWAY:
		returnBike();
		break;
		case BIKE_AWAY_LOCKED:
		unlockTemporyStop();
		break;
		default:
		switchLed(COLOUR_OFF);
		break;
	}
	switchLed(COLOUR_OFF);
	printf("STATE:%d\n\r",bike_state);
	printf("Go to sleep\n\r");
	sleepNow();
}
void readInitialButtonStates(){
	delay(400);
	if(digitalRead(BUTTON_PIN1) == LOW && digitalRead(BUTTON_PIN2) == LOW && bike_state==BIKE_IN_TERMINAL){
		disfunction_signalized=true;
	}
	else if(digitalRead(BUTTON_PIN1) == LOW && bike_state==BIKE_AWAY){
		bike_state=BIKE_AWAY_LOCKED;
	}
}
void returnBike(){
	bike_state=BIKE_IN_TERMINAL;
	switchLed(COLOUR_GREEN);
	delay(1000);
}
void withdrawBike(){
	//bool terminal_in_range = rf_core->rangeTest();
	//rf_core->debug();
bool terminal_in_range = true;
	if(terminal_in_range) {
		printf("Terminal in range!\n\r");
		switchLed(COLOUR_WHITE);
		enableRFID();
		delay(500);
		bool rfid_user_received=getRFID();
		disableRFID();
		if (!rfid_user_received) {
			printf("No RFID given\n\r");
			switchLed(COLOUR_RED);
			delay(2000);
		} else {
			printf("Get RFID : ");
			for(int i =0; i<6;i++){
				Serial.print(client_rfid[i],HEX);
			}
			printf("\n\r");
			switchLed(COLOUR_BLUE);

			if(!getUserCode()) {
				printf("No user code given\n\r");
				switchLed(COLOUR_RED);
				delay(2000);
			} else {
				printf("User code given : ");

				for(int i =0; i<USER_CODE_LENGTH;i++){
					Serial.print(user_code[i]);
				}
				printf("\n\r");
				printf("%i\n\r", user_code_byte);
				switchLed(COLOUR_YELLOW);


				//if(canWithdraw()) {
				delay(3000);
				if(true) {
					printf("Ok withdraw\n\r");
					switchLed(COLOUR_GREEN);
					enableServoMotor();
					openLock();
					delay(3000);
					closeLock();
					disableServoMotor();
					delay(500);
					bike_state=BIKE_AWAY;
				} else {
					printf("No withdraw\n\r");
					switchLed(COLOUR_RED);
					delay(3000);
				}
			}
		}
	}
}
void unlockTemporyStop(){
	switchLed(COLOUR_ORANGE);
	delay(1000);
	switchLed(COLOUR_BLUE);
	if(!getUserCode()) {
		printf("No user code given\n\r");
		switchLed(COLOUR_RED);
	} else {
		printf("User code given : ");

		for(int i =0; i<USER_CODE_LENGTH;i++){
			Serial.print(user_code[i]);
		}
		printf("\n\r");

		printf("%i\n\r", user_code_byte);
		//if(canWithdraw()) {
		if(verifyUserCode()) {
			printf("Ok withdraw\n\r");
			switchLed(COLOUR_GREEN);
			enableServoMotor();
			openLock();
			delay(3000);
			closeLock();
			disableServoMotor();
			delay(500);
			bike_state=BIKE_AWAY;
		} else {
			printf("No withdraw\n\r");
			switchLed(COLOUR_RED);
			delay(3000);
		}
	}
}
bool verifyUserCode(){
	bool correct=true;
	for(int i =0; i<USER_CODE_LENGTH;i++){
		correct &= correct_user_code[i]==user_code[i];

	}
	return correct;
}
void switchLed(int color) {
	int r,g,b;
	r=g=b=0;
	switch(color){
		case COLOUR_RED:
		r = 255;
		break;
		case COLOUR_YELLOW:
		r = 255;
		g = 255;
		break;
		case COLOUR_ORANGE:
		r = 255;
		g = 128;
		break;
		case COLOUR_GREEN:
		g=255;
		break;
		case COLOUR_BLUE:
		b=255;
		break;
		case COLOUR_WHITE:
		r=g=b=255;
		break;

		default:
		case COLOUR_OFF:
		r=g=b= 0;
	}
	//printf("LED COLOR %d - R: %d G: %d B: %d \n\r",color,r,g,b);
	pixels.setPixelColor(0, pixels.Color(r,g,b));
	pixels.show();
}
void blinkLed(int number_of_blink,int color,int timing){
	for(int i=0;i<number_of_blink;i++){
		switchLed(color);
		delay(timing);
		switchLed(COLOUR_OFF);
		delay(timing);
	}
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
	return received;
}


void enableRFID() {
	digitalWrite(RFID_ACTIVATE, HIGH);
}

void disableRFID() {
	digitalWrite(RFID_ACTIVATE, LOW);
}

void enableServoMotor(){
		digitalWrite(MOTOR_ACTIVATE, HIGH);
}
void disableServoMotor(){
		digitalWrite(MOTOR_ACTIVATE, LOW);
}

/********************/
/* LOCKER FUNCTIONS  */
/********************/

void actionLock(bool open) {
	if(open){
		locker_motor.write(5);              // tell servo to go to position in variable 'pos'
		delay(1000);
	}
	else{
		locker_motor.write(90);              // tell servo to go to position in variable 'pos'
		delay(1000);
	}
}


void openLock() {
	actionLock(true);
}

void closeLock() {
	actionLock(false);
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
		switchLed(COLOUR_BLUE);
		if(button_state1== HIGH && button_state2 == HIGH){ // if both buttons are released
			buttons_released = true;
			//digitalWrite(BUTTON_PIN1,LOW);
			//digitalWrite(BUTTON_PIN2,LOW);
		}
		//check if user has released the buttons and if only one is pressed
		else if(buttons_released && (button_state1== LOW ^ button_state2 == LOW))
		{
			// verify which button is pressed and if not both are pressed
			if (button_state1 == LOW && button_state2 == HIGH) {
				printf("B1!\n\r");
				user_code[index_key] = 0;
				index_key++;
				switchLed(COLOUR_OFF);
				user_code_byte = user_code_byte | 0X0;
				if(index_key != USER_CODE_LENGTH) {
					user_code_byte = user_code_byte << 1;
				}
			}
			else if(button_state2 == LOW && button_state1 == HIGH){
				printf("B2!\n\r");
				switchLed(COLOUR_OFF);
				user_code[index_key] = 1;
				index_key++;

				user_code_byte = user_code_byte | 0X1;
				if(index_key != USER_CODE_LENGTH) {
					user_code_byte = user_code_byte << 1;
				}
			}
			buttons_released = false;
			delay(DEBOUNCE_DELAY); //to avoid contact bounce
		}
	}
	return (index_key == USER_CODE_LENGTH);
}

boolean canWithdraw() {
	if(rf_core->handShake()) {
		printf("Success handshake!\n\r");

		uint8_t user_datas[8];
		for(int i =0; i<6;i++){
			user_datas[i]=client_rfid[i];
		}
		user_datas[6]= user_code_byte;
		user_datas[7]= 0;
		//user_datas[7]= '\O';

		while(true){
			rf_core->sendPacket(user_datas);
			delay(200);
		}

		//int response = rf_core->withdrawPermission(user_code_byte, &client_rfid);
		int response = 0;
		printf("response : %i\n\r", response);

		rf_core->closeSession();

		return response == 1;
	}
	return false;
}


/********************/
/* SLEEP FUNCTIONS  */
/********************/

ISR(PCINT1_vect) { //WARNING: comment the ISR(PCINT1_vect) definition in SofwareSerial.cpp
	switchButtonsInterrupts(false); //disable interrupt on buttons, arduino is awake, that avoid the interrupt to fire after wake.
}
/**
* Set the bike in sleep mod. The bike is awaked when a button is pressed.
*/

void sleepNow() {
	//http://donalmorrissey.blogspot.com.es/2010/04/putting-arduino-diecimila-to-sleep.html
	switchButtonsInterrupts(true); //activate interupt on button
	delay(100);
	rf_core->powerDownRadio();
	// Choose our preferred sleep mode:
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	//
	//interrupts();
	//
	// Set sleep enable (SE) bit:
	sleep_enable();
	//

	sleep_mode();
	//
	// Upon waking up, sketch continues from this point.
	rf_core->powerUpRadio();
	sleep_disable();
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
