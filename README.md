

![Polytech](http://www.polytechnice.fr/jahia/jsp/jahia/templates/inc/img/polytech_nice-sophia.png)

Ce projet est réalisé dans le cadre de la formation de prépa intégrée de Polytech'Nice Sophia




# STROBOGLOVE
projet arduino time control glove 

<img src="https://i.makeagif.com/media/3-14-2017/wE9W5J.gif" width="600" height="300">
<!--https://i.makeagif.com/media/1-31-2017/7Gys2-.gif pour une meilleur qualité mais qui marche pas avec l'HTML-->


## Présentation du projet
L'objectif de ce projet est de réaliser une paire de gants capable de "contrôler" le temps grâce à l'effet stroboscopique.

voici un exemple de rendu: 

![time control glove](https://i.makeagif.com/media/1-11-2018/cPzc6O.gif)

Nous allons utiliser deux gants en communication bluetooth; l'un se chargera des flashs lumineux et l'autre se chargera du contrôle.

## Liste du matériel

### Structure : 

* Gant
* Pâte thermique 
* Lentilles optiques, voir les caractéristiques sur la page instructables. Angle de diffusion 60°   Intensité lumineuse en lumens ? 
* Plaque en metal (bon conducteur thermique : cuivre)
	
### Electronique : 

* (Micro-controlleurs (à mettre sur un des deux gants)) 
* Arduino (1 ou 2 ?) 
* Accelerométre 
* Potentiométre 
* Resistances 100 et 10k
* MOSFET (doit supporter 3A)
* lex Sensors 
* LED blanche 100W (Alimenté par ~34V --> ~3A )
	
### Alimentation : 

* Batterie lithium 11,1V - 12V   1500 mAh (plus de préference)
* 2éme (batterie moins puissante)
* Convertisseur Boost DCDC   12V compris dans l'input, 34V compris dans l'output
	
### Transfert de données : Bluetooth ?  
