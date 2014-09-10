#include <Arduino.h>


#include <SPI.h>
//for nrf docs http://nrqm.ca/nrf24l01/
#include "nRF24L01.h"
#include "RF24.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   9
#define CSN_PIN 10

/**PIN MAP

GND	 GND
3V3	 3V3
CE	 9
CSN	 10
SCK	 13
MOSI	 11
MISO	 12
**/

#define STACK_SIZE 6
#define PACKET_SIZE 7 // size of the data part, 7 bytes: [NUM_PACKET,7 bytes of data]


#define TIMEOUT_DELAY 2000
#ifndef RFCore_h
#define RFCore_h



class RFCore
{
  public:
    RFCore(uint64_t,bool);
    void reset();
    bool handShake();
    uint64_t getRemoteID();
    void sendPacket(unsigned char *packet);
    bool getNextPacket(unsigned char *packet);
    void printSerialBuffers();
    void toDebug();
  private:
    void retransmissionQuery(unsigned char pck_number);
    static void messageReceived(void);
    void changeChannel(int new_channel);
    void addTXPacket(unsigned char *new_packet,int num_packet);
    void getTXPacket(unsigned char *packet,int num_packet);
};

#endif
