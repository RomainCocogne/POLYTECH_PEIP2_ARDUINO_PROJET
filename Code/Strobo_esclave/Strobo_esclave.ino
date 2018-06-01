/*
 * Programme sur le gant gauche.
 * 
 * recoit la vitesse de clignotement par bluetooth et fait clignoter la led
 * 
 */

#include <AltSoftSerial.h>
AltSoftSerial BTSerial; 

String message="";          //periode de clignotement en String
int dureeSombre=1;        //periode de clignotement en int (500 par defaut)
const int dureeFlash = 200; // Durée du flash en µs 
const int led_pin = 13;      //pin de la led
bool receiving_data=false;  //recoit ou non la frequence de clignotement
//sauvegarde de millis()
unsigned long timer_blink = 0; //timer pour l allumage de la led
bool led_state=LOW;          //si la led est allumee

void setup() 
{
  //Serial.begin(9600);    //debug
  BTSerial.begin(9600);  //communication bluetooth
  BTSerial.write("AT+BAUD4\r\n");
  pinMode(led_pin,OUTPUT);   
}

void loop()
{
  //**************bluetooth**************//
  char receive;
  if (BTSerial.available()){
    receive=BTSerial.read();
    //Serial.write(receive);
    if(receive=='$')
      BTSerial.print("$");   //test de connection
    else{
      if(receive != '\r' && receive !='\n'){ //si on n'est pas en fin de ligne
        if(receiving_data)                   //si on doit enregistrer les donnee
          message+=receive;                  //ajouter le char au string
        if(receive=='&')                     //si debut de ligne
          receiving_data=true;               //mode enregistrement active
      }
      else {
        receiving_data=false;               //on n enregistre plus rien
        //Serial.println(message);            //debug
        if(message.toInt() !=0)             //on verifie si tout le message est convertible
          dureeSombre=message.toInt();      //convertion en int
        message=""; 
    }
    }
  }

  //**************LED**************//
  led_state = HIGH;  // turn it on
  if(dureeSombre!=1)
      digitalWrite(led_pin, led_state);   // Update the actual LED
  delayMicroseconds(dureeFlash);
    led_state = LOW;  // Turn it off
    digitalWrite(led_pin, led_state);  // Update the actual LED
    delayMicroseconds(dureeSombre);

}


