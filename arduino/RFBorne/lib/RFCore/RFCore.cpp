#include "RFCore.h"



unsigned char tx_data[STACK_SIZE*PACKET_SIZE];
unsigned char rx_data[STACK_SIZE*PACKET_SIZE];

RF24 radio(9,10);


unsigned char data[PACKET_SIZE+1]; // the char arrays to write and read data through radio, 7 bytes of payload, one of PCK_ID
int PAYLOAD_SIZE = sizeof(data); //size in bits of the payload

uint64_t local_id = 0;
volatile uint64_t remote_id = 0;

//for channel preferences: http://nrqm.ca/nrf24l01/firmware/rf_ch-register/
const unsigned char CHANNEL_BROADCAST = 101;
const unsigned char CHANNEL_COM = 115;
unsigned char channel;
bool bc_initializer = false;

int tx_index=0;
int rx_index=0; //index of the last packet received and correctly read.
volatile bool packets_status[STACK_SIZE];

//DEBUG VARIABLES
volatile int received_packet_counter = 0;
volatile int packet_missed_counter=0;
String pck_seq="RECEIVED PACKET SEQ:";
String data_rcv_seq="RECEIVED DATA SEQ:";
String data_send_log="DATA SEND LOG:";
//End debug

RFCore::RFCore(uint64_t id, bool bc_in) //we could dynamically allocate arrays to enlarge lib capacity
{

  radio.begin();
  for(int i =0; i<STACK_SIZE*PACKET_SIZE; i++){
    tx_data[i]=0; //initializing the TX array
    rx_data[i]=0; //initializing the RX array
  }
  for(int i =0; i<STACK_SIZE; i++){
    packets_status[i]=false;
  }
  local_id=id;
  radio.setPALevel(RF24_PA_MAX);
  // 8 bits CRC
  radio.setCRCLength( RF24_CRC_8 ) ;
  // Enabling hardware ACK
  //radio.enableAckPayload();
  radio.setAutoAck(false);

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  //set the payload for the chip
  radio.setPayloadSize(PACKET_SIZE+1);
  radio.setDataRate(RF24_250KBPS);

  changeChannel(CHANNEL_BROADCAST);
  radio.startListening();
  attachInterrupt(0, RFCore::messageReceived, FALLING);  // look for rising edges on digital pin 2
  bc_initializer = bc_in;
  //for pipes explanations:http://maniacalbits.blogspot.be/2013/04/rf24-avoiding-rx-pipe-0-for-enhanced.html
  if(bc_initializer){
    radio.openWritingPipe(local_id); // For broadcasting, RF card can have it's own address. Once the other has it, it can target it.
  }
  else radio.openWritingPipe(0x000000000000); // For receiving broadcasting handshake, card remain on local address 0. All bike wait terminal born on address 0.
  radio.openReadingPipe(1,0x000000000000);


}

void RFCore::sendPacket(unsigned char *packet){
  radio.stopListening();
  //IMPORTANT:putting the RF chip in txmode avoid an rx interrupt, resulting in inchorent data in data
  //(interruption of the for)
  data[0]=tx_index; //the number of packet
  for (int i=0; i<PACKET_SIZE; i++) {
    tx_data[(tx_index*PACKET_SIZE)+i]=packet[i]; //backup the packet in the tx_data stack
    data[i+1] = packet[i];
    //packet[i]=0; don't know if we must do that...
    data_send_log += String(data[i+1]);
  }
  tx_index++;
  bool ok = radio.write( &data, PAYLOAD_SIZE);
  ; //get back in reception. RX interruptions occur again.
  delay(100); // to let the time at the other arduino to treat the datas
}

bool RFCore::getNextPacket(unsigned char *packet){
  int initial_time = millis();
  int current_time = initial_time;
  while(!packets_status[rx_index] && (current_time - initial_time)<TIMEOUT_DELAY){ //while packet not received and timeout not reached
    current_time = millis();
    retransmissionQuery(rx_index);
    delay(100); // wait the packet
  }
  if(!packets_status[rx_index]) return false; //packet not yet arrived, time out, return false

    for(int i = 0;i<PACKET_SIZE;i++){
      packet[i]=rx_data[(rx_index*PACKET_SIZE)+i];
    }
    rx_index++; // the packet is correctly transmit to the program.
    return true;
  }

  void RFCore::retransmissionQuery(unsigned char pck_number){
    Serial.print("ASKING for retransmission:");
    Serial.println(pck_number);
    radio.stopListening();
    data[0]=255; //retransmission query code
    data[1]=pck_number;
    for(int i = 2;i<PACKET_SIZE;i++){
      data[i] = 0;
    }
    bool ok = radio.write( &data, PAYLOAD_SIZE);
    radio.startListening();
  }

  void RFCore::addTXPacket(unsigned char *new_packet,int num_packet){
    for(int i = 0;i<PACKET_SIZE;i++){
      tx_data[(num_packet*PACKET_SIZE)+i]=new_packet[i];
    }
  }
  void RFCore::getTXPacket(unsigned char *packet,int num_packet){
    for(int i = 0;i<PACKET_SIZE;i++){
      packet[i]=tx_data[(num_packet*PACKET_SIZE)+i];
    }
  }

  void RFCore::reset(){
    for(int i =0; i<STACK_SIZE*PACKET_SIZE; i++){
      tx_data[i]=0; //reset buffer arrays
      rx_data[i]=0;
    }
    for(int i =0; i<STACK_SIZE; i++){
      packets_status[i]=false; //reset ACK
    }
    changeChannel(CHANNEL_BROADCAST);
    radio.startListening();
    if(bc_initializer){
      radio.openWritingPipe(local_id);// For broadcasting, RF card can have it's own adress. Once the other has it, it can target it.
    }
    else radio.openWritingPipe(0); // For receiving broadcasting handshake, card remain on local address 0. All bike wait terminal born on address 0.
    radio.openReadingPipe(1,0);
    remote_id=0;
    tx_index=0;
    rx_index=0;

    /*Reset debug var*/
    received_packet_counter = 0;
    packet_missed_counter=0;
    pck_seq="RECEIVED PACKET SEQ:";
    data_rcv_seq="RECEIVED DATA SEQ:";
    data_send_log="DATA SEND LOG:";
  }


  bool RFCore::handShake(){
    if (channel == CHANNEL_COM) return true; //if we've switched channel, it means that the handshake is done
      if(remote_id !=0){
        radio.openReadingPipe(1,remote_id);
        if(!bc_initializer){ // it's the bike, it has received the terminal ID. It's its turn to send local address.
          radio.stopListening();
          radio.write(&local_id,PAYLOAD_SIZE); // Send his bike ID to the terminal
          radio.openWritingPipe(local_id); //bike can now be targeted by the terminal
        }
        changeChannel(CHANNEL_COM); //both change channel
        radio.startListening(); //the passive mode, RX.
        return true;
      }
      changeChannel(CHANNEL_BROADCAST); //no connection established
      if(bc_initializer){ //if it's the terminal
        radio.stopListening(); //in transmission for broadcasting
        radio.write(&local_id,PAYLOAD_SIZE);//terminal broadcast its ID
        radio.startListening();//in reception, wait for bike ID
        delay(50);
      }
      else{ //it's the bike
      radio.startListening();//in reception, wait for bike ID
      delay(50);
    }
    return false;
  }

  /*DEBUG FONCTIONS*/

  uint64_t RFCore::getRemoteID(){
    return remote_id;
  }

  void RFCore::printSerialBuffers(){
    Serial.print("TX BUFFER:");
    for(int i =0; i<STACK_SIZE*PACKET_SIZE; i++){
      Serial.print(tx_data[i]);
      Serial.print(' ');
    }
    Serial.println(' ');
    Serial.print("RX BUFFER:");
    for(int i =0; i<STACK_SIZE*PACKET_SIZE; i++){
      Serial.print(rx_data[i]);
      Serial.print(' ');
    }
    Serial.println(' ');
  }

  void printBooleanACK(){
    Serial.print("BOOL ACK:");
    for(int i =0; i<STACK_SIZE; i++){
      Serial.print(packets_status[i]);
      Serial.print(' ');
    }
    Serial.println(' ');
  }

  void RFCore::changeChannel(int new_channel){
    channel = new_channel;
    radio.setChannel(new_channel);
    delay(100);
  }


  void RFCore::messageReceived(void){
    if(radio.available()){

      if (channel == CHANNEL_BROADCAST)
        {
          uint64_t received_address;
          radio.read(&received_address,PAYLOAD_SIZE); // the bike received the terminal ID.
          remote_id = received_address;

        }
        else if (channel == CHANNEL_COM){
          radio.read(&data,PAYLOAD_SIZE);
          int packet_number = data[0];
          //255 is reserved to announce a missing packet.
          if(packet_number != 255){
            received_packet_counter++; // for debug and stats

            for (int i=0; i<PACKET_SIZE; i++) {
              rx_data[(packet_number*PACKET_SIZE)+i]=data[i+1];
              data_rcv_seq += String(data[i+1]);
            }
            pck_seq += String(packet_number);
            packets_status[packet_number] = true;
          }
          else{
            //the other board ask for a packet retransmission
            packet_missed_counter++; //for statistics
            int pck_number_queried = data[1]; //format is retransmit code, packet to resend
            radio.stopListening();
            data[0] = pck_number_queried;
            for(int i = 0;i<PACKET_SIZE;i++){
              data[i+1]=tx_data[(pck_number_queried*PACKET_SIZE)+i];
            }
            radio.write(&data,PAYLOAD_SIZE);
          }
        }
      }
      radio.startListening();
    }

    void RFCore::toDebug(){ //WARNING: this functions introduce strange effect, see history file.
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
      Serial.println(received_packet_counter);
      Serial.print("Packet missed received queries counter:");
      Serial.println(packet_missed_counter);
      printSerialBuffers();
      printBooleanACK();
      Serial.println(pck_seq);
      Serial.println(data_rcv_seq);
      Serial.println(data_send_log);
      Serial.println("--------------------------");
    }
    /*END DEBUG*/
