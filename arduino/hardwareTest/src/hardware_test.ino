#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

char test_mode='0';


//RF Variables
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL }; // Radio pipe addresses for the 2 nodes to communicate.
byte counter = 1;

void setup()
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
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
    case '2': //RFID Test
    while(!stopTest()){
      printf("*** RFID Test mode ***\n\r");
    }
    break;
    case '3':
    printf("*** LED Test mode ***\n\r");
    break;
  }
}

bool stopTest(){
  if (Serial.available() && toupper(Serial.read()) == 'S'){
    test_mode = '0';
    return true;
  }
}

void RFTest(){
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  delay(200);
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
