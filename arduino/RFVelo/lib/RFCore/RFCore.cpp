#include "RFCore.h"

unsigned char tx_data[STACK_SIZE*PACKET_SIZE];
unsigned char rx_data[STACK_SIZE*PACKET_SIZE];

Nrf2401 Radio;

unsigned int local_id = 0;
volatile unsigned int remote_id = 0;

unsigned char CHANNEL_BROADCAST = 0;
unsigned char CHANNEL_COM = 30;

bool bc_initializer = false;

int tx_index=0;
int rx_index=0; //index of the last packet received and correctly read.
volatile bool packets_status[STACK_SIZE];

RFCore::RFCore(unsigned int id, bool bc_in) //we could dynamically allocate arrays to enlarge lib capacity
{
  for(int i =0; i<STACK_SIZE*PACKET_SIZE; i++){
    tx_data[i]=0; //initializing the array
  }
  for(int i =0; i<STACK_SIZE; i++){
    packets_status[i]=false;
  }
  local_id=id;
  Radio.dataRate = 0;
  changeChannel(CHANNEL_BROADCAST);
  Radio.rxMode(1);
  attachInterrupt(0, RFCore::messageReceived, RISING);  // look for rising edges on digital pin 2
  bc_initializer = bc_in;
  if(bc_initializer){
    Radio.localAddress = local_id; // For broadcasting, RF card can have it's own adress. Once the other has it, it can target it.
  }
  else Radio.localAddress = 0; // For receiving broadcasting handshake, card remain on local address 0. All bike wait terminal born on address 0.
  Radio.remoteAddress = 0;
}

void RFCore::sendPacket(unsigned char *packet){
  Radio.txMode(NRF2401_BUFFER_SIZE); //packet size for 6max char, plus one byte of packet number
  //IMPORTANT:putting the RF chip in txmode avoid an rx interrupt, resulting in inchorent data in Radio.data
  //(interruption of the for)
  Radio.data[0]=tx_index; //the number of packet
  for (int i=0; i<PACKET_SIZE; i++) {
    tx_data[(tx_index*PACKET_SIZE)+i]=packet[i]; //backup the packet in the tx_data stack
    Radio.data[i+1]=packet[i];
  }
  tx_index++;
  Serial.print("pck send number:");
  Serial.println(Radio.data[0]);
  Radio.write();
  Radio.rxMode(NRF2401_BUFFER_SIZE); //get back in reception. RX interruptions occur again.
}

bool RFCore::getNextPacket(unsigned char *packet){
  int initial_time = millis();
  int current_time = initial_time;
  while(!packets_status[rx_index] && (current_time - initial_time)<TIMEOUT_DELAY){ //while packet not received and timeout not reached
    current_time = millis();
    retransmissionQuery(rx_index);
    delay(50); // wait the packet
  }
  if(!packets_status[rx_index]) return false; //packet not yet arrived, time out, return false

  for(int i = 0;i<PACKET_SIZE;i++){
    packet[i]=rx_data[(rx_index*PACKET_SIZE)+i];
  }
  rx_index++;
  return true;
}

void RFCore::retransmissionQuery(unsigned char pck_number){
  Radio.txMode(2);
  Radio.data[0]=255; //retransmission query code
  Radio.data[1]=pck_number;
  Radio.write();
  Radio.rxMode(NRF2401_BUFFER_SIZE);
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

void RFCore::empty(){
  for(int i =0; i<STACK_SIZE*PACKET_SIZE; i++){
    tx_data[i]=0; //initializing the array
  }
}

bool RFCore::handShake(){ // return true if handshaking is established
  if (Radio.channel == CHANNEL_COM) return true; //if we've switched channel, it means that the handshake is done
  if(remote_id !=0){
    Radio.remoteAddress = remote_id;
    if(!bc_initializer){ // it's the bike, it has received the terminal ID. It's its turn to send local address.
    Radio.txMode(1);
    Radio.data[0] = local_id;
    Radio.write(); // Send his bike ID to the terminal
    Radio.localAddress = local_id; //bike can now be targeted by the terminal
  }
  changeChannel(CHANNEL_COM); //both change channel
  Radio.rxMode(NRF2401_BUFFER_SIZE); //the passive mode, RX.
  return true;
}
changeChannel(CHANNEL_BROADCAST); //no connection established
if(bc_initializer){ //if it's the terminal
Radio.txMode(1); //in transmission for broadcasting
Radio.write(local_id);//terminal broadcast its ID
Radio.rxMode(1);//in reception, wait for bike ID
delay(50);
}
else{ //it's the bike
Radio.rxMode(1); //in reception for the terminal ID.
delay(50);
}
return false;
}

/*DEBUG FONCTIONS*/

int RFCore::getRemoteID(){
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

volatile int received_packet_counter = 0;
volatile int packet_missed_counter=0;
String suitepck="suitepck:";

void RFCore::toDebug(){
  Serial.println("-------RF DEBUG-----------");
  Serial.print("Radio channel:");
  Serial.println(Radio.channel);
  Serial.print("Radio mode:");
  Serial.println(Radio.mode);
  Serial.print("Local address:");
  Serial.println(Radio.localAddress);
  Serial.print("Remote address:");
  Serial.println(Radio.remoteAddress);
  Serial.print("Packet received counter:");
  Serial.println(received_packet_counter);
  Serial.print("Packet missed counter:");
  Serial.println(packet_missed_counter);
  printSerialBuffers();
  printBooleanACK();
  Serial.println(suitepck);
  Serial.println("--------------------------");
}
/*END DEBUG*/
void RFCore::changeChannel(int new_channel){
  if(Radio.channel != new_channel){
    Radio.channel = new_channel;
    delay(250);
  }
}


void RFCore::messageReceived(void){
  if(Radio.available()){
    Radio.read();
    if (Radio.channel == CHANNEL_BROADCAST)
    {
      remote_id = Radio.data[0]; // the bike received the terminal ID.
    }
    else if (Radio.channel == CHANNEL_COM){
      int packet_number = Radio.data[0];
      if(packet_number != 255){ //255 is reserved to announce a missing packet.
        received_packet_counter++;
        for (int i=0; i<PACKET_SIZE; i++) {
          rx_data[(packet_number*PACKET_SIZE)+i]=Radio.data[i+1];
        }
        String thisString = String(packet_number);
        suitepck += thisString;
        packets_status[packet_number] = true;
      }
      else{ //the other board ask for a packet retransmission
        packet_missed_counter++; //for statistic
        int pck_number_queried = Radio.data[1]; //format is retransmit code, packet to resend
        Radio.txMode(NRF2401_BUFFER_SIZE);
        Radio.data[0] = pck_number_queried;
        for(int i = 0;i<PACKET_SIZE;i++){
          Radio.data[i+1]=tx_data[(pck_number_queried*PACKET_SIZE)+i];
        }
        Radio.write();
      }
    }
  }
  Radio.rxMode(NRF2401_BUFFER_SIZE);
}
