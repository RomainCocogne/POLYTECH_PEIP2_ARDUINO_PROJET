/*
 * programme sur le gant droit
 * 
 * utilise l accelerometre et les flex sensors pour determiner la frequence de clignotement
 * et l envoie par bluetooth
 */

#include <avr/pgmspace.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
const PROGMEM int PIN = 5;
const PROGMEM int NUMPIXELS = 18;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include <SoftwareSerial.h>
SoftwareSerial BTSerial(11, 10);

#include <Wire.h>  // Library qui sert à comminuquer avec le I²C de l'accelerométre 
#include "Kalman.h" //Classe Kalman qui permet de d'utiliser le filtre de Kalman


//******variables sur les LED*****//
int vitesse_chenille = 20; int vitesse_blink = 50; 
unsigned long timer_led = 0; // sauvegarde la derniere valeur de millis() pour faire le delay
bool aller=true; //blink aller/retour
int i=0;  //position de la chenille de led dans la matrice de neopixels
int couleur[]={1,20,1}; //pour du vert
//********************************//


//*******variables accelerometre*******//
/* Adresse du I²C */
const PROGMEM uint8_t IMUAddress = 0x68;   //Ox annonce un nombre en hexa

/* Variables pour l'accelerométre */ 
int16_t accX; /*Entiers sur 16 bit signés --> Valeurs entre -32768 et +32767 */  
int16_t accY;
int16_t accZ;
int16_t gyroZ;
float accZangle;
float gyroZrate;
float gyroZangle = 180;
uint8_t data[14]; //Stocke les 14 valeurs renvoyées par l'accéléromètre
long lastchange;
boolean actif;
boolean changing_mode;
double last_angle = 0;

/* Variables pour le filtre de Kalman */
Kalman kalmanX;
Kalman kalmanZ;
double kalAngleZ;
uint32_t timer_acc; /* Entier sur 32 bits non signé --> Valeur entre 0 et 2^32 -1 */
//****************************//

//*********variables flex sensor**************//
int flex_0; //Données des flex sensors
int flex_1;
//*******************************************//

//*********variables de delay**********//
const PROGMEM int timer_connection_test = 1000; // temps d'attente pour chaque test de connection 
const PROGMEM int time_send_data= 200;
//sauvegarde de millis()
unsigned long timer_send = 0; unsigned long timer_receive = 0; //pour envoyer et recevoir le test de connection
unsigned long timer_try = 0; //pour essayer de se connecter
unsigned long timer_send_donnee=0;
//************************************//


//**************variables bluetooth***************//
String message="";   //message reçus par le bluetooth
bool is_connected=false; //si les modules sont connectés
bool is_send=false;
int erreurs=0;
//***********************************************//

/*Valeurs pour le mapping (un peu au pif, j'avoue)*/
const PROGMEM int t_macro_min = 11000;     // µs 
const PROGMEM int t_macro_max = 13000;    // µs 
const PROGMEM int t_micro_min = 0;
const PROGMEM int t_micro_max = 500;   // µs
/* Variables finales, celles à envoyer par bluetooth */
int t_macro = 1;   //µs          
int t_micro = 0;   //µs



void setup() {
  /*de base*/
  //Serial.begin(9600); //Pour le debug
  //***************accelerometre**************//
  /* Setup pour lire les données de l'accelerométre */
  Wire.begin();  
  i2cWrite(0x6B, 0x00); /* Sort le I²C du sleep mode --> active l'accelerométre */
  kalmanZ.setAngle(180);  /* Valeur fixée de l'angle au lancement du programme --> fixée à 180° (valeur quand la main est droite) */
  /* Setup du timer. Permet de calculer dt pour le filtre de Kalman */
  timer_acc = micros();
  //*****************************************//
  
  //*********strip led**********************//
  pixels.begin(); // This initializes the NeoPixel library.
  //***************************************//

  //*********bluetooth*********************//
  BTSerial.begin(9600);  // HC-05 default speed in AT command more
  BTSerial.write("AT+BAUD4\r\n");
  //***************************************//

}

void loop() {

  //Serial.println(F(kalAngleZ));

  //******************bluetooth+led**********************//
  
  char receive;         //sauvegarde le char reçus
  if (BTSerial.available()){
    receive=BTSerial.read();
  }
  unsigned long currentMillis = millis();
    if (currentMillis - timer_send_donnee >= time_send_data) {
        timer_send_donnee = currentMillis;
        if(t_micro+t_macro !=0){
          BTSerial.print("&");
          BTSerial.print(t_micro+t_macro);
          BTSerial.print("\n");
        }
    }
  test_connection("send",'$',' '); //test si on est connecté
  test_connection("receive",'$',receive); //test si on est connecté
  
  if(!is_connected){ //si on est pas connecté
    try_connection(); //on essaye
    chenillard(); //petit chenillard
  }
  else{  //sinon
    low_blink(); //on allume tout
  }


 //******************accelerometre+flex**************************//
/* Recupére les données de l'accelerométre */
  mesure_angle();

  if (accZ > 25000 && (millis() - lastchange > 500)) {  
    actif = !actif;                                     
    lastchange = millis();
    t_macro=1; t_micro=0;
  }

  if (actif){
    kalAngleZ = constrain(kalAngleZ - 180,-120,120);   /* Decale l'angle de 180° (donc on est à 0° quand la main est droite)  et limite l'angle à 120° de chaque coté (arbitraire, on peut agrandir le range si on veut)*/ 
   /* Recupére les données des flex sensors */
    flex_0 = analogRead(0);
    flex_1 = analogRead(1);
    /*Si au moins un des deux flex sensors sont pliés --> Reglage macro*/ 
    if ((flex_0 >960) || (flex_1 >900)){  /*La barre du flex_ est plus haute que l'autre car c'est celui qui est un peu plié par defaut */ 
      t_macro = map(kalAngleZ , -120+last_angle , 120+last_angle ,t_macro_min,t_macro_max);
      //Serial.print(F("Mode : macro  ;  "));
      couleur[0]=3; couleur[1]=1; couleur[2]=5;  
    }
    
    /* Sinon --> Reglage micro */
    else {
      t_micro = map(kalAngleZ,-120,120,t_micro_min,t_micro_max);
      last_angle = kalAngleZ;
      //Serial.print(F("Mode : micro  ;  ")); 
      couleur[0]=1; couleur[1]=5; couleur[2]=1;     
      }
      
  
      
      
  }
 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//**********************************************************************************************************************//
//                                                                                                                      //
//                                                   bluetooth                                                          //
//                                                                                                                      //
//**********************************************************************************************************************//

void try_connection(){
  unsigned long currentMillis = millis();
  if (currentMillis - timer_try >= timer_connection_test) {
      timer_try = currentMillis;
    BTSerial.write("AT+INQ\r\n");
    //delay(5000);
    BTSerial.write("AT+CONN1\r\n");
    //BTSerial.write('$');
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




//**********************************************************************************************************************//
//                                                                                                                      //
//                                               Strip led                                                              //
//                                                                                                                      //
//**********************************************************************************************************************//

void all_led_on(){
  for(int j=0; j<=NUMPIXELS; j++){
    pixels.setPixelColor(j, pixels.Color(couleur[0],couleur[1],couleur[2]));     
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
    pixels.setPixelColor(j, pixels.Color(couleur[0]*i,couleur[1]*i,couleur[2]*i));     
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
    if (j-taille<0) pixels.setPixelColor(NUMPIXELS+j, pixels.Color(couleur[0],couleur[1],couleur[2]));
    pixels.setPixelColor(j, pixels.Color(couleur[0],couleur[1],couleur[2]));     
  }
  pixels.setPixelColor(i+1, pixels.Color(0,0,0)); 
}




//**********************************************************************************************************************//
//                                                                                                                      //
//                                               accelerometre                                                          //
//                                                                                                                      //
//**********************************************************************************************************************//

/*   
 *  Fonction de calcul de l'angle, besoin d'un peu plus de detail mais fonctionne 
*/
void mesure_angle() {
  unsigned long currentMicros = micros();
  i2cRead(0x3B); //acquisition des données, demande à l'accelerométre d'envoyer 14 entrées 
  accX = ((data[0] << 8) | data[1]);
  accY = ((data[2] << 8) | data[3]);
  accZ = ((data[4] << 8) | data[5]);
  gyroZ = ((data[12] << 8) | data[13]);
  
  //Calcul de l'angle avec la valeur de g sur z , la vitesse angulaire de z, et le filtre de Kalman
  accZangle = (atan2(accX, accY) + PI) * RAD_TO_DEG;
  gyroZrate = -((double)gyroZ / 131.0);
  gyroZangle += kalmanZ.getRate() * ((double)(currentMicros - timer_acc) / 1000000); // Calculate gyro angle using the unbiased rate
  kalAngleZ = kalmanZ.getAngle(accZangle, gyroZrate, (double)(currentMicros - timer_acc) / 1000000); // Calculate the angle using a Kalman filter
  timer_acc = currentMicros;
}


/* Cette fonction initialise la communication avec le I²C. Elle lui envoie l'ordre de se reveiller et de commencer à transmettre les données 
 * 
 * Pins de l'arduino pour communiquer avec un I²C :
 *      SDA : PC4   --> Serial Data Line, ligne de transfert de données 
 *      SCL : PC5   --> Serial Clock Line, ligne de synchronisation d'horloge  
 * 
 */
void i2cWrite(uint8_t registerAddress, uint8_t data) { 
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  Wire.write(data);
  Wire.endTransmission(); 
}


/* Cette fonction permet de recuperer les données de l'accelerometre. Elle renvoie un tableau avec toutes les données (14 en tout) dans l'ordre d'aquisition.
 *
 * Le tableau contient les données sous la forme suivante :
 * 
 * [accX_H , accX_L , accY_H , accY_L , accZ_H , accZ_L , temperature_H , temperature_L , 
 * gyroX_H , gyroX_L , gyroY_H , gyroY_L , gyroZ_H , gyroZ_L ]
 * 
 * Chaque entrée du tableau est un entier signé sur 8 bits. Chaque donnée (par exemple accX) et séparée en deux entrées (_H et _L)  
 *  L'entrée _H correspond aux 8 premier bits 
 *  L'entrée _L correspond aux 8 derniers bits
 *  
 *  Au final, chaque donnée est un entier signé sur 16 bits. 
 * Comme le I²C envoie toujours les données dans le même ordre, on stocke toutes les données dans un tableau et on extrait celles qui nous interressent 
 *   
 * Beaucoup d'infos interressantes sur la lecture des données sur cette page : https://www.i2cdevlib.com/forums/topic/4-understanding-raw-values-of-accelerometer-and-gyrometer/
 */
void i2cRead(uint8_t registerAddress) { //
  uint8_t nbytes = 14;
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(false); // Don't release the bus
  Wire.requestFrom(IMUAddress, nbytes); // Send a repeated start and then release the bus after reading
  for (uint8_t i = 0; i < nbytes; i++)
    data [i] = Wire.read();

}

