#include "printf.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Rfid.h"
#include <Servo.h>
#include "Adafruit_NeoPixel.h"


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

RF24 radio(RF_PIN_CE,RF_PIN_CSN);
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL }; // Radio pipe addresses for the 2 nodes to communicate.
byte counter = 1;

//RFID
Rfid rfid(RFID_DATA);
byte rfid_data[6] = {0,0,0,0,0,0};

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
const int DEBOUNCE_DELAY =200;
void setup()
{
  pinMode(RFID_ACTIVATE,OUTPUT);
  locker_motor.attach(MOTOR_PIN_CONTROL);  // attaches the servo on pin 9 to the servo object
  pinMode(MOTOR_ACTIVATE,OUTPUT);
  pinMode(BUTTON_PIN1,INPUT_PULLUP);
  pinMode(BUTTON_PIN2,INPUT_PULLUP);
  //pinMode(RFID_ACTIVATE,OUTPUT);
  pixels.begin();
  Serial.begin(57600);
  printf_begin();
  printMenu(true);
  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.openWritingPipe(pipes[1]);        // Both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();                 // Start listening

}

void loop()
{
  if ( Serial.available() )
  {
    test_mode = Serial.read();
  }
  switch(test_mode){
    case '1': //RF Test
    printf("*** RF Test mode ***\n\r");
    radio.printDetails();
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    while(!stopTest()){
      RFTest();
    }
    break;
    case '2': //RFID Test
    printf("*** RFID Test mode ***\n\r");
    digitalWrite(RFID_ACTIVATE,HIGH);
    while(!stopTest()){
      RFIDTest();
    }
    digitalWrite(RFID_ACTIVATE,LOW);
    break;
    case '3':
    printf("*** Servo Test mode ***\n\r");
    digitalWrite(MOTOR_ACTIVATE,HIGH);
    while(!stopTest()){
      servoTest();
    }
    digitalWrite(MOTOR_ACTIVATE,LOW);
    break;
    case '4':
    printf("*** LED Test mode ***\n\r");
    while(!stopTest()){
      LEDTest();
    }
    break;
    case '5':
    printf("*** Buttons Test mode ***\n\r");
    while(!stopTest()){
      buttonsTest();
    }
    break;
  }
}

bool stopTest(){
  if (Serial.available() && toupper(Serial.read()) == 'S'){
    test_mode = '0';
    printMenu(false);
    return true;
  }
  else return false;
}

void RFTest(){
  radio.stopListening();                                  // First, stop listening so we can talk.
  printf("Now sending %d as payload. ",counter);
  byte gotByte;
  unsigned long time = micros();                          // Take the time, and send it.  This will block until complete
  if (!radio.write( &counter,1)){
    printf("failed.\n\r");
  }else{
    if(!radio.available()){
      printf("Blank Payload Received\n\r");
    }else{
      while(radio.available() ){
        unsigned long tim = micros();
        radio.read( &gotByte, 1 );
        printf("Got response %d, round-trip delay: %lu microseconds\n\r",gotByte,tim-time);
        counter++;
      }
    }
  }
  // Try again later
  delay(1000);
}

void RFIDTest(){
  boolean data_received=rfid.RFIDRead(&rfid_data[0]);
  if(data_received){
    printf("rfid received \n\r");
    String data_string=rfid.byteArrayToString(&rfid_data[0],6);
    printf("%s\n\r",data_string.c_str());
    //printf("%s\n\r","data_string");
  }
}

void servoTest(){
  locker_motor.write(5);              // tell servo to go to position in variable 'pos'
  delay(1000);                       // waits 15ms for the servo to reach the position
  locker_motor.write(90);              // tell servo to go to position in variable 'pos'
  delay(1000);                       // waits 15ms for the servo to reach the position
  //locker_motor.write(180);              // tell servo to go to position in variable 'pos'
  //delay(1000);
}

void LEDTest(){
  for(int i=0;i<=6;i++){
    setLed(i);
    delay(500);
  }
}
void buttonsTest(){
  int button_state1=LOW,button_state2=LOW;
  boolean buttons_released=false;
  button_state1 = digitalRead(BUTTON_PIN1);
  button_state2 = digitalRead(BUTTON_PIN2);
  printf("Button 1 state:%i, button 2 state:%i \n\r",button_state1,button_state2);
  /*if(button_state1== LOW && button_state2 == LOW){ // if both buttons are released
			buttons_released = true;
			digitalWrite(BUTTON_PIN1,LOW);
		}
		//check if user has released the buttons and if only one is pressed
		else if(buttons_released && (button_state1== HIGH ^ button_state2 == HIGH))
		{
			// verify which button is pressed and if not both are pressed
			if (button_state1 == HIGH && button_state2 == LOW) {
			  printf("BUTTON 1 PRESSED.");
			}
			else if(button_state2 == HIGH && button_state1 == LOW){
			printf("BUTTON 2 PRESSED.");
			}
			buttons_released = false;
			delay(DEBOUNCE_DELAY); //to avoid contact bounce
		}*/
}
void printMenu(boolean first_display){
  if (first_display){
    printf("\n\r\n\r");
    printf("--- Bikino Hardware Full Stack Test (v0.1) ---\n\r");
    //String logo="           ,@        `;@@         \n\r            '         `,           \n\r            @         @\n\r           ,#:`       #\n\r           @@ ,@@#,  '\n\r        `` ; #    `;@@   `\n\r      @+###+  @     .;'@+##++\n\r    `+######@ .'    @ #######@\n\r    #########' +.   ;#########+\n\r   ,##########  @  # @#########\n\r   @##########   @ @  .@+#####+`\n\r   @##########`  ,+@@    `,::.\n\r   ;##########   :#@:@\n\r    +########@    `  #;      :@\n\r    '#######+        '#     @+\n\r     +######`         '@   ##`\n\r       ;@#.               @,\n\r       \n\r";
    //printf("%s\n\r",logo.c_str());
    printf("%s\n\r","           ,@        `;@@         \n\r            '         `,           \n\r            @         @\n\r           ,#:`       #\n\r           @@ ,@@#,  '\n\r        `` ; #    `;@@   `\n\r      @+###+  @     .;'@+##++\n\r    `+######@ .'    @ #######@\n\r    #########' +.   ;#########+\n\r   ,##########  @  # @#########\n\r   @##########   @ @  .@+#####+`\n\r   @##########`  ,+@@    `,::.\n\r   ;##########   :#@:@\n\r    +########@    `  #;      :@\n\r    '#######+        '#     @+\n\r     +######`         '@   ##`\n\r       ;@#.               @,\n\r       \n\r");
    printf("---           Available modes:           ---\n\r");
  }
  printf("<1> RF mode. \n\r");
  printf("<2> RFID mode. \n\r");
  printf("<3> Servo mode. \n\r");
  printf("<4> LED RGB mode. \n\r");
  printf("<5> Button mode. \n\r");
  printf("<S> Stop the test and return to menu \n\r");
  printf("---        Press <mode> and enter        ---\n\r");
}
void setLed(int color) {
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
	printf("LED COLOR %d - R: %d G: %d B: %d \n\r",color,r,g,b);
	pixels.setPixelColor(0, pixels.Color(r,g,b));
	pixels.show();
}
