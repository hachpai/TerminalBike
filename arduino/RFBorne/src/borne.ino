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
  rf_core = new RFCore(0, true);
}

void loop(){
/*  bool terminal_in_range = rf_core->handShake();
  if(terminal_in_range)
  {
    printf("Terminal in range!\n\r");
  }*/

  //printf("waiting bike\n\r");
  //srf_core->rangeTestCounter();
  rf_core->printSessionCounter();

  //delay(200);
}
