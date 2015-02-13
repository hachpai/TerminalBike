#include "RFCore.h"


RF24 radio(9,10);

//const uint64_t start_bike_pipe = [0xBBBBABCD71LL,0xBBBBABCD71LL+1];
#define range_test_pipes_def 0xBBBBABCD01LL //0xBBBBABCD71LL
#define handshake_pipe_def 0xBBBBABCD03LL
#define start_bike_pipe 0xBBBBABCD05LL

struct Pipe {
  uint64_t terminal;
  uint64_t bike;
}range_test_pipes, handshake_pipes;

//const uint64_t handshake_pipe = [0x544d52687CLL,0x544d52687CLL+1];

bool is_terminal;

char range_successful_ack[]  = "OK";

int id;

RFCore::RFCore(int _id, bool _is_terminal) //we could dynamically allocate arrays to enlarge lib capacity
{
  range_test_pipes.terminal = range_test_pipes_def;
  range_test_pipes.bike = range_test_pipes_def+ 1;
  handshake_pipes.terminal = handshake_pipe_def;
  handshake_pipes.bike = handshake_pipe_def+1;

  id = _id;
  is_terminal = _is_terminal;
  // Setup and configure rf radio
  radio.begin();
  radio.setAutoAck(1); // Ensure autoACK is enabled
  //radio.enableAckPayload(); // Allow optional ack payloads
  //radio.setRetries(0,15); // Smallest time between retries, max no. of retries
  //8*8 bytes of data, meaning 8 integers
  radio.setPayloadSize(sizeof(uint64_t));
  radio.setRetries(15,15);
  if(!is_terminal) {
    radio.openWritingPipe(range_test_pipes.terminal);
    radio.openReadingPipe(1,range_test_pipes.bike);
    radio.openReadingPipe(2,start_bike_pipe+id);
  } else {
    radio.openWritingPipe(range_test_pipes.bike);
    radio.openReadingPipe(1,range_test_pipes.terminal);
    radio.openReadingPipe(2,handshake_pipes.terminal);
  }

  if(is_terminal){
    //radio.writeAckPayload(range_test_pipes.terminal,&range_successful_ack, sizeof(range_successful_ack) );
    radio.startListening(); // Start listening
  }
  radio.printDetails(); // Dump the configuration of the rf unit for debugging
}

bool RFCore::sendPacket(unsigned char *packet){
  //radio.stopListening();
  //IMPORTANT:putting the RF chip in txmode avoid an rx interrupt, resulting in inchorent data in data
  //(interruption of the for)

  //radio.startWrite( &data[0], sizeof(unsigned char) ,0);
  // to let the time at the other arduino to treat the datas
}

bool RFCore::getPacket(unsigned char *packet){
}


void RFCore::endOfSession(){
  //reset everything
}


bool RFCore::handShake(){
  if (!is_terminal)
  {
    char data_received[3];
    radio.stopListening();
    radio.openWritingPipe(handshake_pipes.terminal);
    if (!radio.write( &id, sizeof(int)))
    {
      printf("write timeout.\n\r");
      //return false;
    }
    else{
      radio.startListening();
      delay(50);
      if(!radio.available())
      {
        printf("Nothing received\n\r");
      }
      else
      {
        while(radio.available())
        {
          radio.read( &data_received, sizeof(data_received));
          if(strcmp(data_received,range_successful_ack)==0)
          {
            printf("Got response %s\n\r",data_received);
            return true;
          }

        }

      }

    }
    return false;

  }
  else
  {
    int data_received;
    radio.startListening();
    delay(50);
    if(!radio.available())
    {
      printf("Nothing received terminal\n\r");
    }
    else
    {
      while(radio.available())
      {
        radio.read( &data_received, sizeof(data_received));

        printf("Got response %s\n\r",data_received);
      }

      radio.stopListening();
      radio.openWritingPipe(start_bike_pipe+data_received);
      if(!radio.write(&range_successful_ack, sizeof(range_successful_ack))){
        printf("bike didn't received\n\r");
      }
      else{
        printf("bike received confirmation code\n\r");
        return true;
      }

    }
    return false;
  }
}

bool RFCore::rangeTest()
{
  if (!is_terminal)
  {
    char ack_received[3];
    radio.stopListening(); // First, stop listening so we can talk.
    printf("Now sending %d as payload. ",id);
    delay(20);
    if (!radio.write( &id, sizeof(int)))
    {
      printf("write timeout.\n\r");
      //return false;
    }

    if(!radio.available())
    {
      printf("Nothing received\n\r");
    }
    else
    {
      while(radio.available())
      {
        radio.read( &ack_received, sizeof(ack_received));
        if(strcmp(ack_received,range_successful_ack)==0)
        {
          printf("Got response %s\n\r",ack_received);
          return true;
        }

      }

    }
    return false;
  }
  else
  {
    uint8_t pipeNo;
    uint64_t bike_id_received;
    byte gotByte; // Dump the payloads until we've gotten everything
    if(!radio.available())
    {
      printf("No data received\n\r");
      return false;
    }
    else
    {
      while( radio.available(&pipeNo))
      {
        radio.read( &bike_id_received, sizeof(int) );
        printf("Got data %d from pipe %u \n\r",bike_id_received,pipeNo);
        radio.writeAckPayload(range_test_pipes.terminal,&range_successful_ack, sizeof(range_successful_ack));
      }
      return true;
    }
  }
}





void RFCore::messageReceived(void){

}

void RFCore::debug(){ //WARNING: this functions introduce strange effect, see history file.
  /*Serial.println("-------RF DEBUG-----------");
  Serial.print("radio channel:");
  Serial.println(radio.channel);
  Serial.print("radio mode:");
  Serial.println(radio.mode);
  Serial.print("Local address:");
  Serial.println(radio.localAddress);
  Serial.print("Remote address:");
  Serial.println(radio.remoteAddress);*/
  radio.printDetails();
  Serial.print("Packet received counter:");

  Serial.print("Packet missed received queries counter:");
  Serial.println("something");

  printf("--------------------------\n\r");
}
/*END DEBUG*/
