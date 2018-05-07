

#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11); 

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


