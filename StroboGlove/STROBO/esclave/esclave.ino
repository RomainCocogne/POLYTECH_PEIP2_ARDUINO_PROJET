

#include <SoftwareSerial.h>

SoftwareSerial BTSerial(11, 10); 

String message="";

void setup() 
{
  pinMode(9, OUTPUT);  
  digitalWrite(9, HIGH); 
  Serial.begin(9600);
  BTSerial.begin(9600);  
}

void loop()
{

  char receive;
  if (BTSerial.available()){
    receive=BTSerial.read();
    //Serial.write(receive);
    if(receive=='$')
      BTSerial.write("$");
    else{
      if(receive != '\r')
        message+=receive;
      else {
        Serial.println(message);
        message="";
    }
    }
  }


  if (Serial.available())
    BTSerial.write(Serial.read());
}


/*
 const int dureeFlash = 200; // Durée du flash en µs  --> a mettre sur l'autre arduino 
const int led_pin = 2;


      t_macro = map(analogRead(0),0,1024,t_macro_min,t_macro_max); 
      digitalWrite(led_pin, HIGH);  
      delayMicroseconds(dureeFlash); 
      digitalWrite(led_pin, LOW);  
      delayMicroseconds(t_macro + t_micro);
      Serial.print("Freq : ");Serial.print(1/((t_macro+ t_micro)*0.000001));Serial.println(" Hz");
 */

