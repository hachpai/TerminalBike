#include "printf.h"
#include "RFCore.h"
#include <Servo.h>
#include <avr/sleep.h>
#include <avr/power.h>

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
  }

  delay(200);
}
