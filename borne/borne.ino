
#include <Nrf2401.h>

Nrf2401 Radio;
volatile unsigned char request_code = 0;
volatile unsigned char bike_id = 0;
volatile unsigned char client_id = 0;
// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only 
// modified by the interrupt handler - a function that we never directly call


unsigned char BORNE_ID=42;
void setup(void)
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  attachInterrupt(0, messageReceived, RISING);  // look for rising edges on digital pin 2
  Radio.channel = 240;
  Radio.dataRate = 0;
  Radio.txMode(1);
}

void loop(void)
{
  //Radio.channel= 111;
  if(request_code==0)
  {
    Serial.println("BROADCASTING");
    Radio.txMode(1);
    Radio.write(BORNE_ID);
    Radio.rxMode(3);
    delay(1000);
  }
  else{
  Serial.print("VARIABLE STATE, RC:");
  Serial.print(request_code);
  Serial.print(",bike_id:"); 
  Serial.print(bike_id);
  Serial.print(",client_id:");
  Serial.print(client_id);
  Serial.println(" END");
  //Radio.channel= 119;
  Radio.txMode(3);
  Radio.data[0] = 27;
  Radio.data[1] = 68;
  Radio.data[2] = 89;
  Radio.write();
  request_code = 0;
  }
}

void messageReceived(void)
{
if(Radio.available()){
      digitalWrite(13, HIGH);
  Radio.read();
  request_code = Radio.data[0];
  bike_id = Radio.data[1];
  client_id = Radio.data[2];
  Radio.txMode(3);
  /*Serial.print("BIKE FOUND");
  Serial.print(bike_id);
  Serial.print("/REQUEST:");
  Serial.print(request_code);*/
}
}

