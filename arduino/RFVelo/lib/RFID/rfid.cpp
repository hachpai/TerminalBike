// digit 2 <-> 9
// 5V <->
// GND <->

#include "rfid.h"

//http://stackoverflow.com/questions/13169714/creating-a-library-for-an-arduino
Rfid::Rfid(int pin)
	: RFIDSerial(pin,pin)
{
	RFIDSerial = SoftwareSerial(pin,pin);
	RFIDSerial.begin(9600);
}

// rFIDRead ->
bool Rfid::RFIDRead(byte *rFIDCode) {
  byte i = 0;
  byte readByte = 0;
  byte tempByte = 0;
  byte checksum = 0;
  byte bytesread = 0;
  if(RFIDSerial.available() > 0) {
    if((readByte = RFIDSerial.read()) == 2) {                  // check for header
      bytesread = 0;
      while (bytesread < 12) {                        // read 10 digit code + 2 digit checksum
        if( RFIDSerial.available() > 0) {
          readByte = RFIDSerial.read();
          if((readByte == 0x0D)||(readByte == 0x0A)||(readByte == 0x03)||(readByte == 0x02)) { // if header or stop bytes before the 10 digit reading
            return false;                                  // stop reading
          }

          // Do Ascii/Hex conversion:
          if ((readByte >= '0') && (readByte <= '9')) {
            readByte = readByte - '0';
          } else if ((readByte >= 'A') && (readByte <= 'F')) {
            readByte = 10 + readByte - 'A';
          }

          // Every two hex-digits, add byte to code:
          if (bytesread & 1 == 1) {
            // make some space for this hex-digit by
            // shifting the previous hex-digit with 4 bits to the left:
            rFIDCode[bytesread >> 1] = (readByte | (tempByte << 4));

          if (bytesread >> 1 != 5) {                // If we're at the checksum byte,
              checksum ^= rFIDCode[bytesread >> 1];       // Calculate the checksum... (XOR)
            };
          } else {
            tempByte = readByte;                           // Store the first hex digit first...
          };

          bytesread++;                                // ready to read next digit
        }
      }

      if (bytesread == 12) {              // if 12 digit read is complete
        if(rFIDCode[5] == checksum) {
          return true;
        }
      }
    }
  }
  return false;
}
// <- fin RFIDRead

// byteArrayToString ->
void Rfid::printHexRFID(byte *byteArray, int byteArraySize) {
  int i;
  String ret = "";
  for (i=0; i<byteArraySize; i++) {
		if(byteArray[i] < 16) printf("0");
		printf("%x",byteArray[i]);
		if (i < 5) printf(":");
    // if(byteArray[i] < 16) ret += "0";
    // ret += String(byteArray[i], HEX);
    // if (i < 4) ret += " ";
  }
  //return ret;
}
// <- fin byteArrayToString
