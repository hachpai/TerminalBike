#include <Arduino.h>
#include <SPI.h>
#include "nRF24L01.h" //for nrf docs http://nrqm.ca/nrf24l01/
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

#ifndef RFCore_h
#define RFCore_h
class RFCore
{
	public:
		RFCore(unsigned int,bool);

		/**
		 * Return true if the bike can communicate with the born/terminal
		 */
		bool handShake();

		/**
		 * Return true if the bike and born/terminal are close enought to communicate
		 */
		bool rangeTest();
		void debug();
		void printSessionCounter();

		/**
		 * Return true if the bike and born/terminal are close enought to communicate
		 */
		void closeSession(); // stop the hand shake (ie. the bike says it has finish to communicate withe the born/terminal)
		void checkRadioNoIRQ();
    bool sendPacket(unsigned char *packet);
    bool getPacket(unsigned char *packet);

    void powerDownRadio();
    void powerUpRadio();

	//	int withdrawPermission(unsigned char, void*);
	private:
		static void check_radio(void);
};
#endif
