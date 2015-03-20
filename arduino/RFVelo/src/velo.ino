#include "printf.h"
#include "RFCore.h"
#include <Servo.h>
#include <avr/sleep.h>
#include <avr/power.h>

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

bool terminal_in_range = false;
RFCore * rf_core;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  int BIKE_ID = 8;//will read EEPROM after
  rf_core = new RFCore(BIKE_ID, false);
}

void loop(){
  terminal_in_range = rf_core->rangeTest();
  if(terminal_in_range)
  {
    printf("Terminal in range!\n\r");
    delay(500);
    if(rf_core->handShake()){

      printf("success handshake!\n\r");
      rf_core->closeSession();
    //  rf_core->printSessionCounter();
    }

  }
  delay(1000);
}
