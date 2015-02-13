#include "printf.h"
#include "RFCore.h"
#include <Servo.h>
#include <avr/sleep.h>
#include <avr/power.h>

RFCore * rf_core;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  int BIKE_ID = 2;//will read EEPROM after
  rf_core = new RFCore(BIKE_ID, true);
}

void loop(){
  bool terminal_in_range = rf_core->handShake();
  if(terminal_in_range)
  {
    printf("Terminal in range!\n\r");
  }

  delay(200);
}
