#include <Arduino.h>
#include "printf.h"
#include "RFCore.h"
#include <Servo.h>
#include <avr/sleep.h>
#include <avr/power.h>
void setup(void);
void loop();
#line 1 "src/borne.ino"
//#include "printf.h"
//#include "RFCore.h"
//#include <Servo.h>
//#include <avr/sleep.h>
//#include <avr/power.h>

RFCore * rf_core;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  int BIKE_ID = 2;//will read EEPROM after
  rf_core = new RFCore(BIKE_ID, true);
}

void loop(){
  bool range = rf_core->rangeTest();
  printf("RANGE TEST: %d \n\r",range);

  delay(200);
}
