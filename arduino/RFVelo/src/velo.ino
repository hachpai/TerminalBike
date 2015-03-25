#include "printf.h"
#include "RFCore.h"
#include <Servo.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "Rfid.h"

/************************/
/** PINs configuration **/
/************************/

/* Button */
const int BUTTON_PIN1 = 14; //A0
const int BUTTON_PIN2 = 15; //A1

/* LED */
const int RED_PIN = 5;
const int GREEN_PIN = 6;
const int BLUE_PIN = 7;

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
const int RFID_IN = 7; //final version will use pin 0 (serial)
const int RFID_OUT = 8;//final version will use pin 1 (without serial.begin, it makes them unusable)


/*others constants*/
const int RFID_TIMEOUT = 10000; //10 sec to put the rfid card

bool terminal_in_range = false;

Rfid rfid(RFID_IN,RFID_OUT);
RFCore * rf_core;

byte client_rfid[6] = {34,35,36,37,38,39};

void setup(void) {
	Serial.begin(57600);

	MCUCR = (1<<ISC01) | (1<<ISC00); //01 and 00 for triggering interrupts on change at low level
  
	printf_begin();

	pinMode(BUTTON_PIN1, INPUT);
	pinMode(BUTTON_PIN2, INPUT);
  
	int BIKE_ID = 8;//will read EEPROM after
	rf_core = new RFCore(BIKE_ID, false);
}

void switchButtonsInterrupts(boolean on) { //TRUE for ON, FALSE for off
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

void testRFID() {
	printf("Test RFID...");
	if(getRFID()) {
			for(int i =0; i<6;i++){
			Serial.print(client_rfid[i],HEX);
		}
	} else {
		printf(" NO RFID\n\r");
	}
}

void loop() {
	terminal_in_range = rf_core->rangeTest();
	if(terminal_in_range) {
		printf("Terminal in range!\n\r");
		testRFID();
		delay(500);
		if(rf_core->handShake()) {
			printf("success handshake!\n\r");
			rf_core->closeSession();
			//  rf_core->printSessionCounter();
		}
	}

	sleepNow();
}
