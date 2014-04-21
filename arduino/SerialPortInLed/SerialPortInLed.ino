char inputString[50]; // Allocate some space for the string
int nextChar = 0;
byte inputByte;
int led = 13;

void setup() {
  Serial.begin(9600);   // connect to the serial port
  pinMode(led, OUTPUT);  
}

void loop() {
  if (Serial.available()) {
    inputByte = Serial.read();
    if(inputByte == '\n') {
      nextChar = 0;
      
      if(inputString[0] == 'O' && inputString[1] == 'l' && inputString[2] == 'a'
       && inputString[3] == ' ' && inputString[4] == 'o' && inputString[5] == 'l'
       && inputString[6] == 'a') {
          digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
          delay(300);               // wait for a second
          digitalWrite(led, LOW);    
       }
    }
    else {
      inputString[nextChar] = inputByte;
      nextChar ++;
    }
  }
}
