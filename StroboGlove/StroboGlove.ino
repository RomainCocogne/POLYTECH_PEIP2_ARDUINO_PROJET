/*
 * Idées :
 * remplacer les float (32 bits) par des int (qui ne prennent que 16 bits) ? 
 * mettre une en-tête avec nos noms et un resumé du projet
 * 
 * Apparamment on peut regler la sensibilité du l'accelerometre, a voir dans la datasheet (https://store.invensense.com/datasheets/invensense/MPU-6050_DataSheet_V3%204.pdf) 
*/

/*Libraries */
#include <Wire.h>  // Library qui sert à comminuquer avec le I²C de l'accelerométre 
#include "Kalman.h" //Classe Kalman qui permet de d'utiliser le filtre de Kalman

/*Constantes */
const int ledPin = 2; 
const int dureeFlash = 200; // Durée du flash en µs
  /*Valeurs pour le mapping*/
const float t_macro_min = 2;     // ms
const float t_macro_max = 10;    // ms 
const float t_micro_max = 500;   // µs


/* Variables finales, celles à envoyer par bluetooth */
float t_macro = 2;   //ms          
float t_micro = 0;   //µs
int flex_0; //Données des flex sensors
int flex_1;
int flex_2;

 

/* Adresse du I²C */
uint8_t IMUAddress = 0x68;   //Ox annonce un nombre en hexa

/* Variables pour l'accelerométre */ 
int16_t accX; /*Entiers sur 16 bit signés --> Valeurs entre -32768 et +32767 */  
int16_t accY;
int16_t gyroZ;
float accZangle;
float gyroZrate;
float gyroZangle = 180;

/* Variables pour le filtre de Kalman */
Kalman kalmanX;
Kalman kalmanZ;
double kalAngleZ;
uint32_t timer; /* Entier sur 32 bits non signé --> Valeur entre 0 et 2^32 -1 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  /* Setup classique */
  Serial.begin(250000); //Pour le debug
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,HIGH);
  
  /* Setup pour lire les données de l'accelerométre */
  Wire.begin();  
  i2cWrite(0x6B, 0x00); /* Sort le I²C du sleep mode --> active l'accelerométre */
  kalmanZ.setAngle(180);  /* Valeur fixée de l'angle au lancement du programme --> fixée à 180° (valeur quand la main est droite) */

  /* Setup du timer. Permet de calculer dt pour le filtre de Kalman */
  timer = micros();
}

void loop() {
  /* Recupére les données de l'accelerométre */
  mesure_angle();
  kalAngleZ = constrain(180 - kalAngleZ,-90,90);   /* Decale l'angle de 180° (donc on est à 0° quand la main est droite)  et limite l'angle à 90° de chaque coté (arbitraire, on peut agrandir le range si on veut)*/ 
  
  /* Recupére les données des flex sensors */
  flex_0 = analogRead(0);
  flex_1 = analogRead(1);
  flex_2 = analogRead(2);
  
  /*Si au moins deux des trois flex sensors sont pliés --> Reglage micro*/ 
  if ((flex_0 >900 && flex_1>1000) || (flex_1 >1000 && flex_2>900) || (flex_0 >900 && flex_2>900)){  /*La barre du flex_1 est plus haute que les autres car c'est celui qui est un peu plié par defaut */ 
    t_micro = map(kalAngleZ,-90,90,0,t_micro_max);
    Serial.print("Mode : micro  ;  ");
  }
  /* Sinon --> Reglage macro */
  else {
    t_macro = map(kalAngleZ,-90,90,t_macro_min,t_macro_max);
    Serial.print("Mode : macro  ;  ");
  }
  
  Serial.print("Freq : ");Serial.print(1/(t_macro*0.001 + t_micro*0.000001));Serial.println(" Hz");  
  
  /*Flash de la LED puis attente */
  digitalWrite(ledPin,HIGH);
  delayMicroseconds(dureeFlash);
  digitalWrite(ledPin,LOW);  
  delay(t_macro);
  delayMicroseconds(t_micro);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*   
 *  Fonction de calcul de l'angle, besoin d'un peu plus de detail mais fonctionne 
*/
void mesure_angle() {
  uint8_t* data = i2cRead(0x3B, 14); //acquisition des données, demande à l'accelerométre d'envoyer 14 entrées  (a quoi sert le "*" ? )
  accX = ((data[0] << 8) | data[1]);
  accY = ((data[2] << 8) | data[3]);
  gyroZ = ((data[12] << 8) | data[13]);
  
  //Calcul de l'angle avec la valeur de g sur z , la vitesse angulaire de z, et le filtre de Kalman
  accZangle = (atan2(accX, accY) + PI) * RAD_TO_DEG;
  gyroZrate = -((double)gyroZ / 131.0);
  gyroZangle += kalmanZ.getRate() * ((double)(micros() - timer) / 1000000); // Calculate gyro angle using the unbiased rate
  kalAngleZ = kalmanZ.getAngle(accZangle, gyroZrate, (double)(micros() - timer) / 1000000); // Calculate the angle using a Kalman filter
  timer = micros();
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
uint8_t* i2cRead(uint8_t registerAddress, uint8_t nbytes) { //
  uint8_t data[nbytes];
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(false); // Don't release the bus
  Wire.requestFrom(IMUAddress, nbytes); // Send a repeated start and then release the bus after reading
  for (uint8_t i = 0; i < nbytes; i++)
    data [i] = Wire.read();
  return data;
}


