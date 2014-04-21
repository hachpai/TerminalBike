char inData[200]; // Allocate some space for the string
int  availableBytesNbr = 0;
byte inByte;
int led = 13;

void setup() {
  Serial.begin(9600);   // connect to the serial port
  pinMode(led, OUTPUT);  
}

void loop() {
  if (Serial.available()) {
    inByte = Serial.read();
    if(inByte == '\n') {
      digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(500);               // wait for a second
      digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW 
    }
  }
  availableBytesNbr = Serial.available();
  if (availableBytesNbr > 0) {
     Serial.readBytes(inData, availableBytesNbr);  
        
     delay(100);
     Serial.print("I got this ->");
     Serial.print(inData);
     Serial.println("<-");
     delay(100);
  }
}
