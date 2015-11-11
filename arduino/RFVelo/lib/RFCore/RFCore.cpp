#include "RFCore.h"

volatile uint8_t in_session=false;
volatile int irq1,irq2,irq3;

volatile int session_counter=0;
RF24 radio(9,10);


const int TIMEOUT_DELAY=3000;

#define range_test_pipe_terminal 0xBBBBABCD01LL //0xBBBBABCD71LL
#define handshake_pipe_terminal 0xBBBBABCD03LL
#define session_pipe_terminal 0xBBBBABCD05LL //for sessions pair to pair communication
#define start_bike_pipe 0xBBBBABCD07LL

bool is_terminal;

char success_code[]  = "H1";
char success_logout_code[]  = "OKLO";
char busy_code[] = "H0";

unsigned int bike_id;
void printPipe();

RFCore::RFCore(unsigned int _id, bool _is_terminal)
{
  bike_id = _id;
  is_terminal = _is_terminal;
  // Setup and configure rf radio
  radio.begin();
  radio.setAutoAck(1); // Ensure autoACK is enabled
  //radio.enableAckPayload(); // Allow optional ack payloads
  //radio.setRetries(0,15); // Smallest time between retries, max no. of retries
  //8*8 bytes of data, meaning 8 integers
  radio.setPayloadSize(sizeof(uint64_t));
  radio.setRetries(15,15);
  radio.maskIRQ(1,1,0);
  radio.setChannel(60);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);

  if(is_terminal) {
    radio.openReadingPipe(1,range_test_pipe_terminal); //for range testing queries
    radio.openReadingPipe(2,handshake_pipe_terminal); //for handshake queries
    radio.openReadingPipe(3,session_pipe_terminal); //for sessions
  } else { //bike
    radio.openWritingPipe(range_test_pipe_terminal); //bike is always the first to talk
    radio.openReadingPipe(1,start_bike_pipe+bike_id);
  }

  if(is_terminal){
    radio.startListening(); // Start listening
  }


  radio.printDetails(); // Dump the configuration of the rf unit for debugging
  if(is_terminal){//only the terminal listen passively to radio
    //attachInterrupt(0, check_radio, LOW);
  }

}



bool RFCore::sendPacket(uint8_t *packet){
  radio.stopListening(); // First, stop listening so we can talk.
  if(is_terminal){
    //terminal need to be in session
    if(!in_session){
      return false;
    }
    radio.openWritingPipe(start_bike_pipe+bike_id);
  }
  else{
    //printf("PACKET SIZE:%d",sizeof(packet));
    //printf("Message of size %d send:",sizeof(uint64_t));
    for(int i =0; i<8;i++){
      printf("%u ",packet[i]);
    }
    printf("\n\r");
    if(in_session){
      radio.openWritingPipe(session_pipe_terminal);
      return radio.write(packet,sizeof(uint64_t));
    }
  }
  return false;
}

bool RFCore::getPacket(unsigned char *packet){
  radio.startListening();
  delay(50);
  bool received=false;
  if(radio.available()){
    received=true;
    radio.read(packet, sizeof(uint64_t));
  }
  radio.stopListening();
  return received;
  //do timeout operations here
}

bool RFCore::handShake(){

  char data_received[3];
  radio.stopListening();
  radio.openWritingPipe(handshake_pipe_terminal);
  bool ping_pong=false;
  printf("Handshaking...");
  int time_start=millis();
  while(!ping_pong)
  {
    int time_now = millis();
    if((time_now-time_start) > TIMEOUT_DELAY){
      printf("TIMEOUT_DELAY\n\r");
      return false;
    }
    radio.stopListening();
    radio.write( &bike_id, sizeof(int));

    radio.startListening();
    delay(20);
    if(!radio.available())
    {
      printf(".");
    }
    else{
      ping_pong=true;
    }
  }
  printf("\n\r");
  while(radio.available())
  {
    radio.read( &data_received, sizeof(data_received));
    printf("Got response in HANDSHAKE %s\n\r",data_received);
    delay(50);
  }
  if(strcmp(data_received,success_code)==0)
  {
    in_session=true;
    return true;
  }
  return false;
}


bool RFCore::rangeTest()
{
  if (!is_terminal)
  {
    radio.stopListening(); // First, stop listening so we can talk.
    radio.openWritingPipe(range_test_pipe_terminal);
    bool ping_pong=false;
    printf("Range testing...");
    int time_start=millis();
    while(!ping_pong)
    {
      int time_now = millis();
      if((time_now-time_start) > TIMEOUT_DELAY){
        printf("TIMEOUT_DELAY\n\r");
        return false;
      }
      radio.stopListening();
      radio.write( &bike_id, sizeof(bike_id));
      radio.startListening();
      delay(20);
      if(!radio.available())
      {

        printf(".");

      }
      else{
        ping_pong=true;
      }

    }
    printf("\n\r");
    if(radio.available())
    {
      char response[3];

      radio.read( &response, sizeof(response));
      if(strcmp(response,success_code)==0 || strcmp(response,busy_code)==0){//terminal reply with a busy or free code, range passed
        printf("Well received in range test:%s\n\r",response);
        while(radio.available())
        {
          char data_received[3];
          radio.read( &data_received, sizeof(data_received));
          printf("Got response in range TEST %s\n\r",data_received);
          delay(50);
        }
        return true;
      }

    }
    return false;
  }
}

void RFCore::closeSession(){
  session_counter++;
  uint8_t closing_session_code[8];
  closing_session_code[0]= 'L';
  closing_session_code[1]= 'O';
  closing_session_code[2]= 'L';
  closing_session_code[3]= '\0';
  printf("closing session...");
  bool ping_pong = false;
  int time_start=millis();
  radio.stopListening();
  radio.openWritingPipe(session_pipe_terminal);

  while(!ping_pong)
  {
    int time_now = millis();
    if((time_now-time_start) > TIMEOUT_DELAY*10){
      printf("TIMEOUT_DELAY SO BAD!!\n\r");
    }
    radio.stopListening();
    radio.write(&closing_session_code,sizeof(closing_session_code));
    radio.startListening();
    delay(20);
    if(!radio.available())
    {

      printf(".");

    }
    else{
      char response[5];
      radio.read(&response,sizeof(response));
      printf("RECEIVED: %s\n\r",response);
      if(strcmp(response,success_logout_code)==0){
        ping_pong=true;
        in_session=false;
        printf("Log out success!!\n\r");
      }

    }
  }
  printf("\n\r");
}

void RFCore::printSessionCounter()
{
  printf("Session Counter: %d , irq1 %d, irq2 %d, irq3:%d\n\r",session_counter,irq1,irq2,irq3);
}


void RFCore::checkRadioNoIRQ(void)
{
  uint8_t pipe_number;

  while(radio.available(&pipe_number))
  {
    printf("data on pipe %u, in session:%d\n\r",pipe_number,in_session);
    //last_pipe= pipeNo;
    switch(pipe_number){

      case 1: //range_test pipe
      if(is_terminal){
        unsigned int _id;
        radio.read(&_id,sizeof(_id));
        radio.stopListening();
        radio.openWritingPipe(start_bike_pipe+_id);
        //printf("id received:%d",id);
        if(in_session){

          radio.write(&busy_code,sizeof(busy_code)); //send the bike that the terminal is busy
        }
        else{ //terminal is busy
          radio.write(&success_code,sizeof(success_code));
        }
        radio.startListening();
      }
      break;
      case 2: //handshake pipe (init session)
      if(is_terminal){
        unsigned int _id;
        radio.read(&_id,sizeof(_id));
        radio.stopListening();
        radio.openWritingPipe(start_bike_pipe+_id);
        if(in_session  && !(_id == bike_id)){ //terminal is busy and a NEW bike is asking for a session
          radio.write(&busy_code,sizeof(busy_code)); //send the bike that the terminal is busy
        }
        else{ //terminal accept a handshake, or the bike didn't receive the confirmation
          if(radio.write(&success_code,sizeof(success_code))){
            bike_id = _id;
            in_session=true;
          }

        }
        radio.startListening();

      }
      break;
      case 3: //session pipe
      uint8_t data[8]; //8 bytes = payload of uint64_t
      /*CODE
      LO = log out, WD = withdraw, RT= return
      packet is [2 bytes CODE,(5 bytes RFID),(byte user code)] = 8 bytes of data (payload size)
      */
      radio.read(&data,sizeof(data));
      //WARNING: check the complete data array for ['L','O']
      if(data[0]=='L' && data[1]=='O'){//for log out code in session
        printf("Received %s,  ACK FOR LO, SEND BACK %s ON PIPE %lu.\n\r",data,success_logout_code,start_bike_pipe+bike_id);
        in_session=false;
        radio.stopListening();
        radio.openWritingPipe(start_bike_pipe+bike_id);
        //printf("id received:%d",id);
        radio.write(&success_logout_code,sizeof(success_logout_code)); //notice successful logout to bike
      }
      else{
        printf("Received user code and rfid:");
        for(int i=0; i<7;i++){
          printf("%u ",data[i]);
        }
        printf("\n\r");
        /*radio.stopListening();
        radio.openWritingPipe(start_bike_pipe+bike_id);
        //printf("id received:%d",id);
        radio.write(&success_code,sizeof(success_code)); //notice successful logout to bike
        //here we receive RFID+user code and check DB. If authorized, send confirmation code for unlocking*/
      }
      break;
    }
    radio.startListening();
    //radio.read( &bike_id_received, sizeof(int) );
  }
  //return true;
}

void RFCore::powerDownRadio(){
  radio.powerDown();
}
void RFCore::powerUpRadio(){
  radio.powerUp();
  delay(10); // up to 5ms to get the chip back to life. 10 for security
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
// void RFCore::check_radio(void)
// {
//
//   bool tx,fail,rx;
//   radio.whatHappened(tx,fail,rx);                     // What happened?
//
//   if(rx){
//     uint8_t pipe_number;
//
//     while( radio.available(&pipe_number))
//     {
//       //last_pipe= pipeNo;
//       switch(pipe_number){
//         case 1: //range_test pipe
//         irq1++;
//         if(is_terminal){
//           int _id;
//           radio.read(&_id,sizeof(int));
//           radio.stopListening();
//           radio.openWritingPipe(start_bike_pipe+_id);
//           //printf("id received:%d",id);
//           if(in_session){
//
//             radio.write(&busy_code,sizeof(busy_code)); //send the bike that the terminal is busy
//             in_session=false;//CHEAT for testing
//           }
//           else{ //terminal is busy
//             radio.write(&success_code,sizeof(success_code));
//           }
//           radio.startListening();
//         }
//         break;
//         case 2: //handshake pipe (init session)
//         irq2++;
//         if(is_terminal){
//           int id;
//           radio.read(&id,sizeof(int));
//           radio.stopListening();
//           radio.openWritingPipe(start_bike_pipe+id);
//           if(in_session){
//             radio.write(&busy_code,sizeof(busy_code)); //send the bike that the terminal is busy
//           }
//           else{ //terminal accept a handshake
//             if(radio.write(&success_code,sizeof(success_code))){
//               in_session=true;
//             }
//
//           }
//           radio.startListening();
//
//         }
//         break;
//         case 3: //session pipe
//         irq3++;
//
//         uint8_t data[8]; //8 bytes = payload of uint64_t
//         /*CODE
//         LO = log out, WD = withdraw, RT= return
//         packet is [2 bytes CODE,(5 bytes RFID),(byte user code)] = 8 bytes of data (payload size)
//         */
//         radio.read(&data,sizeof(data));
//         if(data[0]=='L' && data[1]=='O'){//for log out code in session
//           in_session=false;
//           session_counter++;
//           radio.stopListening();
//           radio.openWritingPipe(start_bike_pipe+bike_id);
//           //printf("id received:%d",id);
//           radio.write(&success_code,sizeof(success_code)); //notice successful logout to bike
//         }
//         break;
//       }
//       radio.startListening();
//       //radio.read( &bike_id_received, sizeof(int) );
//     }
//     //return true;
//   }
// }
/*bool tx,fail,rx;
radio.whatHappened(tx,fail,rx);                     // What happened?

// If data is available, handle it accordingly
if ( rx ){

if(radio.getDynamicPayloadSize() < 1){
// Corrupt payload has been flushed
return;
}
// Read in the data
uint8_t received;
radio.read(&received,sizeof(received));
// If this is a ping, send back a pong
if(received == ping){
radio.stopListening();
// Can be important to flush the TX FIFO here if using 250KBPS data rate
//radio.flush_tx();
radio.startWrite(&pong,sizeof(pong),0);
}else
// If this is a pong, get the current micros()
if(received == pong){
round_trip_timer = micros() - round_trip_timer;
Serial.print(F("Received Pong, Round Trip Time: "));
Serial.println(round_trip_timer);
}
}
// Start listening if transmission is complete
if( tx || fail ){
radio.startListening();
Serial.println(tx ? F("Send:OK") : F("Send:Fail"));
}*/
