/*
rfid.h - Library for RFID reader through serial port.
Released into the public domain.
*/
#ifndef Rfid_h
#define Rfid_h

#include <Arduino.h>
#include <SoftwareSerial.h>

class Rfid
{
public:
  Rfid(int pin);
  bool serialReadLine(String &dest);
  bool RFIDRead(byte *rFIDCode);
  String byteArrayToString(byte *byteArray, int byteArraySize);
private:
  SoftwareSerial RFIDSerial;
};

#endif
