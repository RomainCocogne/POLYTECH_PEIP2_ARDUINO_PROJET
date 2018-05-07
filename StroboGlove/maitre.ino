#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#define PIN            6 
#define NUMPIXELS      30
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include <SoftwareSerial.h>
SoftwareSerial BTSerial(10, 11);

//******variables sur les LED*****//
int vitesse_chenille = 20; int vitesse_blink = 50; 
unsigned long timer_led = 0; // sauvegarde la derniere valeur de millis() pour faire le delay
bool aller=true; //blink aller/retour
int i=0;  //position de la chenille de led dans la matrice de neopixels
//********************************//

//*********variables de delay**********//
int timer_connection_test = 1000; // temps d'attente pour chaque test de connection 
//sauvegarde de millis()
unsigned long timer_send = 0; unsigned long timer_receive = 0; //pour envoyer et recevoir le test de connection
unsigned long timer_try = 0; //pour essayer de se connecter
//************************************//

String message="";   //message reçus par le bluetooth
bool is_connected=false; //si les modules sont connectés
bool is_send=false;
int erreurs=0;



void setup() {
  pixels.begin(); // This initializes the NeoPixel library.

  pinMode(9, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(9, HIGH); 
  Serial.begin(9600);
  BTSerial.begin(9600);  // HC-05 default speed in AT command more
 // BTSerial.write("AT+INQ\r\n");
  //BTSerial.write("AT+CONN1\r\n");

}

void loop() {
  
  char receive;         //sauvegarde le char reçus
  if (BTSerial.available()){
    receive=BTSerial.read();
    if(receive !='$')
      Serial.write(receive); //ecrit le char 
    /*
    if(receive != '\r')
      message+=receive;
    else {
      Serial.println(message);
      message="";
    }*/
  }
  if (Serial.available()){
    BTSerial.write(Serial.read());
    //message="";
  }
  test_connection("send",'$',' '); //test si on est connecté
  test_connection("receive",'$',receive); //test si on est connecté
  
  if(!is_connected){ //si on est pas connecté
    try_connection(); //on essaye
    low_blink(); //on allume tout
  }
  else{  //sinon
    chenillard(); //petit chenillard
  }
 
}

//**************BLUETOOTH**************//

void try_connection(){
  unsigned long currentMillis = millis();
  if (currentMillis - timer_try >= timer_connection_test) {
      timer_try = currentMillis;
    BTSerial.write("AT+INQ\r\n");
    //delay(5000);
    BTSerial.write("AT+CONN1\r\n");
  }
}


void test_connection(String mode, char test, char receive){
  unsigned long currentMillis = millis();
  
    if(mode=="send" && !is_send){
      if (currentMillis - timer_send >= timer_connection_test) {
    timer_send = currentMillis;
    timer_receive=currentMillis;
      BTSerial.write(test);
      is_send=true;
      }
    } 
    
    else if(mode=="receive" && is_send){
      if(receive==test){
        erreurs=0;
        is_connected= true;
        is_send=false;
      }
      else if (currentMillis - timer_receive >= timer_connection_test) {
        timer_receive = currentMillis;
        //timer_send = currentMillis;
        if(erreurs>5){
          is_connected= false;
          erreurs=0;
        }
        is_send=false;
        erreurs++;
      }
    }
}





//***************LED***************//

void all_led_on(){
  for(int j=0; j<=NUMPIXELS; j++){
    pixels.setPixelColor(j, pixels.Color(1,20,1));     
  }
  pixels.show();
}

void low_blink(){
  unsigned long currentMillis = millis();
  if (currentMillis - timer_led >= vitesse_blink) {
      timer_led = currentMillis;
  if(i>10) aller = false;
  if(i<2) aller = true;
  if(aller) i++;
  else i--;
  for(int j=0; j<=NUMPIXELS; j++){
    pixels.setPixelColor(j, pixels.Color(i,20*i,i));     
  }
  pixels.show();
  }
}

void chenillard(){
  unsigned long currentMillis = millis();
  if (currentMillis - timer_led >= vitesse_chenille) {
      timer_led = currentMillis;
      chenille (i,7);
      chenille (i-15,7);
      pixels.show(); // This sends the updated pixel color to the hardware.
      if(i>NUMPIXELS-1) i=0;
      else i++;
   }
}

void chenille(int i,int taille){
  if(i-taille<0) pixels.setPixelColor(NUMPIXELS-taille+i, pixels.Color(0,0,0));
  pixels.setPixelColor(i-taille, pixels.Color(0,0,0)); 
  for(int j=i-taille+1; j<=i; j++){
    if (j-taille<0) pixels.setPixelColor(NUMPIXELS+j, pixels.Color(1,20,1));
    pixels.setPixelColor(j, pixels.Color(1,20,1));     
  }
  pixels.setPixelColor(i+1, pixels.Color(0,0,0)); 
}





