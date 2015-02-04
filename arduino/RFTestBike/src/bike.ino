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

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };


const unsigned int WAITING_SELECTION = 0;
const unsigned int SELECTED = 1;

//unsigned int designation_attemp = 0;

unsigned int mode = WAITING_SELECTION;
unsigned long bike_id = 12;
//unsigned long terminal_id = 0;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rROLE: BIKE \n\r");

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

  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}


void loop(void)
{
  if (mode == WAITING_SELECTION) {
    printf("WAITING_SELECTION\n\r");
    if (radio.available()) {
      bool done = false;
      unsigned long id_received;
      while (!done) {
        done = radio.read( &id_received, sizeof(unsigned long) );
        // Spew it
        printf("Bike : got id %lu...\n\r",id_received);
        delay(20);
      }

      if(id_received == 0) { // par la suite dans un certain rang
        // First, stop listening so we can talk
        radio.stopListening();

        // Send the final one back.
        radio.write( &bike_id, sizeof(unsigned long) );
        printf("Bike : Please choose me (id: %lu )\n\r",bike_id);

        // Now, resume listening so we catch the next packets.
        radio.startListening();
      } else if (id_received == bike_id) {
        printf("Bike : YOUHOU! I've been selected.\n\r");
        mode = SELECTED;
      } else {
        printf("Bike : Fuck it's not me...\n\r");
      }
    }
    delay(200); 
  } else {
    unsigned long code = 12;

    // First, stop listening so we can talk
    radio.stopListening();

    // Send the final one back.
    radio.write( &code, sizeof(unsigned long) );
    printf("Bike : The code is %lu...\n\r",code);

    // Now, resume listening so we catch the next packets.
    radio.startListening();

    mode = WAITING_SELECTION;
  }
  delay(200);
}