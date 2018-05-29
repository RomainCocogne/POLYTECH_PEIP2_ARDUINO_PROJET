/*
 * Programme sur le gant gauche.
 * 
 * recoit la vitesse de clignotement par bluetooth et fait clignoter la led
 * 
 */

#include <SoftwareSerial.h>
SoftwareSerial BTSerial(11, 10); 

String message="";          //frequence de clignotement en String
int dureeSombre=1;        //frequence de clignotement en int (500 par defaut)
const int dureeFlash = 200; // Durée du flash en µs 
const int led_pin = 13;      //pin de la led
bool receiving_data=false;  //recoit ou non la frequence de clignotement
//sauvegarde de millis()
unsigned long timer_blink = 0; //timer pour l allumage de la led
bool led_state=LOW;          //si la led est allumee

void setup() 
{
  Serial.begin(9600);    //debug
  BTSerial.begin(9600);  //communication bluetooth
  BTSerial.write("AT+BAUD4\r\n");
  pinMode(led_pin,OUTPUT);
  pinMode(9, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(9, HIGH);   
}

void loop()
{
  //**************bluetooth**************//
  char receive;
  if (BTSerial.available()){
    receive=BTSerial.read();
    //Serial.write(receive);
    if(receive=='$')
      BTSerial.write("$");   //test de connection
    else{
      if(receive != '\r' && receive !='\n'){ //si on n'est pas en fin de ligne
        if(receiving_data)                   //si on doit enregistrer les donnee
          message+=receive;                  //ajouter le char au string
        if(receive=='&')                     //si debut de ligne
          receiving_data=true;               //mode enregistrement active
      }
      else {
        receiving_data=false;               //on n enregistre plus rien
        Serial.println(message);            //debug
        if(message.toInt() !=0)             //on verifie si tout le message est convertible
          dureeSombre=message.toInt();      //convertion en int
        message=""; 
    }
    }
  }

  //**************LED**************//
   unsigned long currentMillis = micros();
   if((led_state == HIGH) && (currentMillis - timer_blink >= dureeFlash))
  {
    led_state = LOW;  // Turn it off
    timer_blink = currentMillis;  // Remember the time
    digitalWrite(led_pin, led_state);  // Update the actual LED
  }
  else if ((led_state == LOW) && (currentMillis - timer_blink >= dureeSombre))
  {
    led_state = HIGH;  // turn it on
    timer_blink = currentMillis;   // Remember the time
    if(dureeSombre!=1)
      digitalWrite(led_pin, led_state);   // Update the actual LED
  }
  //Serial.print("Freq : ");Serial.print(1/((dureeSombre)*0.000001));Serial.println(" Hz");
}


