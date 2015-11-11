#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include "RFCore/RFCore.h"
using namespace std;

bool radioNumber = 1;
RFCore * rf_core;

/********************************/

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};
const char *socket_path = "\0terminal";

int no_w_bytes,no_r_bytes;
int server_sockfd,io_sockfd;

bool write_socket(void *bufw, int datasize);
bool read_socket(void *bufr,int datasize);

int main(int argc, char** argv){

  /*SOCKET CONF*/
  struct sockaddr_un addr;


  if ((server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    printf("socket error");
    exit(-1);
  }
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

  unlink(socket_path);

  if (bind(server_sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    printf("bind error");
    exit(-1);
  }

  if (listen(server_sockfd, 5) == -1) {
    printf("listen error");
    exit(-1);
  }
  /*END SOCKET CONF*/
  //blocking operation, waiting for connections
  printf("Waiting for node.js...\n\r");
  if ((io_sockfd = accept(server_sockfd, NULL, NULL)) == -1) {
    perror("accept error");
  }
  printf("Node.js connected.\n");
  rf_core = new RFCore(0, true);
  rf_core->debug();
  bool server_already_responded=false;
  unsigned char server_response[8];
  while(true){

    rf_core->checkRadioNoIRQ();
    if(!rf_core->inSession()){
      server_already_responded=false;
    }
    if(rf_core->inSession() && rf_core->dataSessionAvailable()){
      printf("In session with bike %d\n\r", rf_core->getBikeId());
      unsigned char received_data[8];
      printf("before getsession.\n\r");

      rf_core->getSessionData(received_data);
      printf("received_data=");
      for(int i=0; i<8;i++){
        printf("%u |",received_data[i]);
      }
      printf("\n\r");

      //let the time to nodejs to contact the server


      if(!server_already_responded){
        write_socket(received_data,8);
        delay(200); //wait nodejs and server response
        read_socket(server_response,8);

        printf("Server response:%c%c",server_response[0],server_response[1]);
        server_response[2]=rf_core->getBikeId();
        printf("\n\r");
      }
      // ROTTEN LINE! int bike_id = std::atoi(( char * ) server_response[2]);
      rf_core->sendPacket(server_response);
      printf("sending to bike:%c%c%d\n\r",server_response[0],server_response[1],server_response[2]);
      // if(strcmp(server_response,"OK") ==0){
      //
      // }
      // else{
      //   //rf_core->sendPacket()
      // }
    }
    delay(50);
  }
  return false;
}
bool write_socket(void *bufw, int data_size){
  //int data_size=sizeof(bufw);
  //printf("DATA SIZE:%d",data_size);
  if ((no_w_bytes=write(io_sockfd, bufw, data_size)) != data_size) {
    if(no_w_bytes>0)
    {
      fprintf(stderr,"partial write:%d",no_w_bytes);
      return false;
    }
    else {
      perror("write error");
      return false;
    }
  }
  else {return true;}
}
bool read_socket(void *bufr, int data_size){
  if((no_r_bytes=read(io_sockfd,bufr,data_size)) > 0) {
    //printf("read %u bytes: %.*s\n", no_r_bytes, no_r_bytes, buf);
    return true;
  }
  if (no_r_bytes == -1) {
    perror("read");
    //exit(-1);
    return false;
  }
  else if (no_r_bytes == 0) {
    printf("EOF\n");
    return false;
  }
  return false;
}
/* working test
int i = 0;
while(true){
char bufw[10];
sprintf(bufw, "number:%d\0", i);
i++;
write_socket(bufw);
sleep(1);
}
*/
/*
================ NRF Configuration ================
STATUS		 = 0x8f RX_DR=0 TX_DS=0 MAX_RT=0 RX_P_NO=7 TX_FULL=1
RX_ADDR_P0-1	 = 0xf7f7f7f7f7 0xffffffef01
RX_ADDR_P2-5	 = 0x03 0x07 0xe7 0xe7
TX_ADDR		 = 0xf7f7f7f7f7
RX_PW_P0-6	 = 0x00 0x0c 0x0c 0x0c 0x00 0x00
EN_AA		 = 0x3f
EN_RXADDR	 = 0x0f
RF_CH		 = 0x3e
RF_SETUP	 = 0x33
CONFIG		 = 0x3f
DYNPD/FEATURE	 = 0x00 0x00
Data Rate	 = 250KBPS
Model		 = nRF24L01+
CRC Length	 = 16 bits
PA Power	 = PA_LOW
*/
