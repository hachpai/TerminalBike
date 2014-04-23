
#include <Nrf2401.h>

Nrf2401 Radio;
volatile unsigned char borne_id = 0;
volatile unsigned char BIKE_ID = 200;
volatile unsigned char client_id = 123;
// note that the "linkEstablished" variable is declared volatile
// this prevents the compiler from optimizing it out as it is only 
// modified by the interrupt handler - a function that we never directly call

void setup(void)
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  attachInterrupt(0, messageReceived, RISING);  // look for rising edges on digital pin 2
  Radio.channel = 240;
  Radio.dataRate = 0;
  Radio.rxMode(1);
}
                     
void loop(void)
{
  //Radio.channel= 111;

  Radio.rxMode(1);
  Serial.println(borne_id);
  if(borne_id==0)
  {
    Serial.print("LISTENING...");
  }
  else{
    Radio.channel = 0;
    delay(200);
    Serial.print("BORNE FOUND! ID:");
    Serial.println(borne_id);
    digitalWrite(13, HIGH);
  
    Radio.txMode(3);
    Radio.data[0] = 37;
    Radio.data[1] = 78;
    Radio.data[2] = 99;
    Radio.write();
    borne_id = 0;
    Serial.println(" END");
    delay(2000);
  }
}

void messageReceived(void)
{
  //Serial.println("RECEIVED"); 
    digitalWrite(13, HIGH);
  Radio.read();
  borne_id = Radio.data[0];
  //Serial.println(borne_id);
  Radio.txMode(3);
}
