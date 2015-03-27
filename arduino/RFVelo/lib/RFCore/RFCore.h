#include <Arduino.h>


#include <SPI.h>
//for nrf docs http://nrqm.ca/nrf24l01/
#include "nRF24L01.h"
#include "RF24.h"


/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   9
#define CSN_PIN 10

/**PIN MAP

2 4 6 8 10
1 3 5 7 9
    _

GND 2->GND
3V3	 1->3V3
CE 5->9
CSN	 10->10
SCK	 9->13
MOSI	 8->11
MISO	 7->12

**/

#define PAYLOAD_SIZE 8 // size of the data part, 9 bytes: [NUM_PACKET,8 bytes of data]
/*Communication sequence:
TERMINAL: broadcast his adress (uint64_t) on channel 1, 8 bytes of data
BIKE: when address received, send his own address (uint64_t)
(Terminal: send autorisation back)
bike send RFID (uint8 *5)
bike send user code (uint8_t)
terminal send YES or NO
*/


#define TIMEOUT_DELAY 2000
#ifndef RFCore_h
#define RFCore_h



class RFCore
{
  public:
    RFCore(unsigned int,bool);
    bool handShake();
    bool rangeTest();
    void debug();
    void printSessionCounter();
    void closeSession();
    void checkRadioNoIRQ();
    bool sendData(const void* buf, uint8_t len);
  private:
    static void check_radio(void);
};

#endif
