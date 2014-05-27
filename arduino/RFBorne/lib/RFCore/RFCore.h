#include <Arduino.h>
#include <Nrf2401.h>
#define STACK_SIZE 6
#define PACKET_SIZE 6
#define TIMEOUT_DELAY 1500
#ifndef RFCore_h
#define RFCore_h



class RFCore
{
  public:
    RFCore(unsigned int,bool);
    void empty();
    bool handShake();
    int getRemoteID();
    void sendPacket(unsigned char *packet);
    bool getNextPacket(unsigned char *packet);
    void printSerialBuffers();
    int getCounter();
  private:
    void retransmissionQuery(unsigned char pck_number);
    static void messageReceived(void);
    void changeChannel(int new_channel);
    void addTXPacket(unsigned char *new_packet,int num_packet);
    void getTXPacket(unsigned char *packet,int num_packet);
};

#endif
