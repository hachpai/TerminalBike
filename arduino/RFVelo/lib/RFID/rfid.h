#include <SoftwareSerial.h>

bool serialReadLine(String &dest);
void serialPrintLine(String s);
bool rFIDRead(SoftwareSerial *RFIDSerial, byte *rFIDCode);
String byteArrayToString(byte *byteArray, int byteArraySize);
