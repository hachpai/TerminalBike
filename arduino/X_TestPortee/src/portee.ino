// Test si le velo est a porte de la borne (dans ce cas le velo recoit un ack)

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const short role_pin = 5; 

typedef enum { role_bike = 1, role_terminal } role_e; // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Bike", "Terminal"}; // The debug-friendly names of those roles
role_e role; 

const uint64_t bike_id = 0xBBBBABCD71LL;

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL }; 

void setup(){
  pinMode(role_pin, INPUT); // set up the role pin
  digitalWrite(role_pin,HIGH); // Change this to LOW/HIGH instead of using an external pin
  delay(20); 

  if ( digitalRead(role_pin) ) // read the address pin, establish our role
    role = role_bike;
  else
  role = role_terminal;

  Serial.begin(57600);
  printf_begin();
  printf("Test portee ack");
  printf("ROLE: %s\n\r",role_friendly_name[role]);

  // Setup and configure rf radio
  radio.begin();
  radio.setAutoAck(1); // Ensure autoACK is enabled
  radio.enableAckPayload(); // Allow optional ack payloads
  //radio.setRetries(0,15); // Smallest time between retries, max no. of retries
  radio.setPayloadSize(sizeof(uint64_t)); // Here we are sending 1-byte payloads to test the call-response speed
  
  if( role == role_bike) {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  } else {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  
  radio.startListening(); // Start listening
  radio.printDetails(); // Dump the configuration of the rf unit for debugging
}


void loop(void) {
  if (role == role_bike) {
    uint64_t bike_id_return;
    unsigned long time = micros();

    radio.stopListening(); // First, stop listening so we can talk.
    printf("Now sending %lu as payload. ",bike_id);

    if (!radio.write( &bike_id, sizeof(uint64_t) )){
      printf("failed.\n\r");
    } else {
      if(!radio.available()){
        printf("Blank Payload Received\n\r");
      } else {
        while(radio.available() ){
          unsigned long tim = micros();
          radio.read( &bike_id_return, sizeof(uint64_t));
          printf("Got response %lu, round-trip delay: %lu microseconds\n\r",bike_id_return,tim-time);
        }
      }
    }

    delay(1000);
  } else {
    byte pipeNo;
    uint64_t bike_id_received;
    byte gotByte; // Dump the payloads until we've gotten everything
    while( radio.available(&pipeNo)){
      radio.read( &bike_id_received, sizeof(uint64_t) );
      printf("Got data %lu \n\r",bike_id_received);
      radio.writeAckPayload(pipeNo,&bike_id_received, sizeof(uint64_t) );
    }
  }
}