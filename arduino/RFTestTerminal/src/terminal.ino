/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

const unsigned int BROADCASTING = 0; // dit qu'elle est libre
//const unsigned int BIKE_DESIGNATION = 1; // dit a un velo qu'elle le choisi
const unsigned int WAITING_BIKE_INFORMATION = 1; // attend les infos du velo

unsigned int mode = BROADCASTING;
unsigned long selected_bike_id;
unsigned int bike_designation_attemp = 0;
unsigned long terminal_id = 0; //borne -> terminal


void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rROLE: BORNE \n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

void send_bike_selection_message()
{
  radio.stopListening();

  // Send the final one back.
  radio.write( &selected_bike_id, sizeof(unsigned long) );
  printf("Terminal : Selection of bike with id %lu...\n\r",selected_bike_id);

  // Now, resume listening so we catch the next packets.
  radio.startListening();

  bike_designation_attemp++;
}

void loop(void)
{
  if (mode == BROADCASTING) {
    printf("Terminal : BROADCASTING\n\r");
    if (radio.available()) {
      bool done = false;
      while (!done) {
        done = radio.read( &selected_bike_id, sizeof(unsigned long) );
        // Spew it
        printf("Terminal : git bike id  %lu...\n\r",selected_bike_id);

        mode = WAITING_BIKE_INFORMATION;
        bike_designation_attemp = 0;
        send_bike_selection_message();

        delay(10);
      }
    } else {
      radio.stopListening();

      // Send the final one back.
      radio.write( &terminal_id, sizeof(unsigned long) );
      printf("Terminal : send id %lu...\n\r",terminal_id);

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
    delay(20);
  } else {
    printf("WAITING_BIKE_INFORMATION\n\r");
    if ( radio.available() ) {
      unsigned long code;
      bool done = false;
      while (!done) {
        done = radio.read( &code, sizeof(unsigned long) );
        // Spew it
        printf("Got payload %lu...\n\r",code);
        delay(20);
      }

      mode = BROADCASTING;
    } else {
      if(bike_designation_attemp > 5) {
        mode = BROADCASTING;
      } else {
        send_bike_selection_message();
      }
    }
  }
  delay(250);
}
