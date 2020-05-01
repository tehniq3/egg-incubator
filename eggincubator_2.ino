/*
EGG INCUBATOR
ver.1m0 - 13.05.2017, Craiova - Romania, Europe, Earth
Nicu FLORICA (niq_ro) write this sketch for Viorel SOSA, Costa Rica, 
using parts of other sketches :)
based on data from http://nicuflorica.blogspot.ro/2015/08/termostat-cu-control-umiditate-2.html
http;//www.arduinotehniq.com/
http://nicuflorica.blogspot.ro/
http://arduinotehniq.blogspot.com/
ver.1.h - add some feature
v.2.0 - add countdown with RTC
*/

#include <Wire.h>
#include "RTClib.h"  // https://github.com/adafruit/RTClib
#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

RTC_DS1307 rtc;

#include <LiquidCrystal_I2C.h> // https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// http://arduino.cc/en/Reference/LiquidCrystalCreateChar
byte grad[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};


// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
#include "DHT.h"      // https://github.com/tehniq3/used_library (DHT)
#define DHTPIN A0     // what pin we're connected to A0
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// if is just sensor:
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);


// for DS18B20 - http://arduinoprojects.ru/2014/08/%D0%BF%D1%80%D0%BE%D1%81%D1%82%D0%BE%D0%B9-%D1%82%D0%B5%D1%80%D0%BC%D0%BE%D1%81%D1%82%D0%B0%D1%82-%D0%BD%D0%B0-arduino-%D0%B8-%D1%86%D0%B8%D1%84%D1%80%D0%BE%D0%B2%D0%BE%D0%BC-%D1%82%D0%B5%D1%80%D0%BC/
#include <OneWire.h>   // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library
#define dspin 10
#define ONE_WIRE_BUS dspin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int resolution = 11 ;  // for more precission put 12

// define pins for relay or leds
#define temppeste A1
#define umidpeste A2
#define motoras 5
#define motoras1 4
#define eroarepin 13

byte motordr = 0 ; //Motor direction used mith dcmotor and hbridge l293d

// define variable
float te, t1, tes, dete, ete;   // variable for temperature
int has, hass, dehas, ehas;        // variable for humiditiy

byte heat;  // is 1 for heater and 0 for cooler
byte dry;   // is 1 for dryer and 0 for wetter 

// part for menu: http://nicuflorica.blogspot.ro/2015/05/termostat-cu-afisaj-led.html
#define BUT1 6    // - switch
#define BUT2 7    // + switch
#define BUT3 8    // MENU switch

byte meniu = 13; // if MENIU = 0 is clasical 
                // if MENIU = 1 is for temperature set (tes)
                // if MENIU = 2 is for dt temperature (dete)
                // if MENIU = 3 is for humidity set (hass)
                // if MENIU = 4 is for dh humidity (dehas)
                // if MENIU = 5 is for hours between turning eggs (hours)
                // if MENIU = 6 is for time turning eggs (seconds)
                // if MENIU = 7 is for error temperature
                // if MENIU = 8 is for error humidity
                // if MENIU = 9 is for time of hatching
                // if MENIU = 10 is for first rotating
                // if MENIU = 11 is for last day rotation
                // if MENIU = 12 is for START the hatching time
                
// http://tronixstuff.com/2011/03/16/tutorial-your-arduinos-inbuilt-eeprom/
#include <EEPROM.h>

int is,im,ih,id,ida;                          // variables for time
float taim,s1,m1,h1,d1;                       //  Set up variables to calculate time

int taim1, taim2;
unsigned long sfrotatii, taim11, taim22;
byte rotit = 0;
byte rotire = 0;
byte eroaret = 0;
byte eroareh = 0;
// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 2000; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

//int knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag

unsigned long intraremeniu;
unsigned long iesiremeniu = 60000; // 
byte anulare = 0;
byte eroare = 0;

int ora1, minut1, secunda1;
float rest1;
unsigned long tpintors;

boolean debug = false; //Serial communication for debuging. Set to true for serial communication.
//boolean debug = true; //Serial communication for debuging. Set to true for serial communication.

int tzile, zi1, zi2;
byte shtart;  

unsigned int timp, timp0, timp1, timp2, dtimp;
int zi, ora, minut, secunda;
unsigned long dzi, dora;
int dminut, dsecunda;
  
void setup()
{
  Serial.begin(9600);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
// initialize the LCD
  lcd.begin();
// create custom symbol
lcd.createChar(0, grad);
  // Turn on the blacklight and print a message.
  lcd.backlight();

// Initialize DHT sensor
  dht.begin();

// for DS18B20 sensor
sensors.begin(); 
// 11 bit resolution by default 
  // Note the programmer is responsible for the right delay
  // we could do something usefull here instead of the delay
  sensors.setResolution(resolution);
  delay(250/ (1 << (12-resolution)));
  
// define outputs
 pinMode(temppeste, OUTPUT);  
 pinMode(umidpeste, OUTPUT);
 pinMode(motoras, OUTPUT);
 pinMode(motoras1, OUTPUT);
 pinMode(eroarepin, OUTPUT);

// set the default state for outputs
  digitalWrite(temppeste, LOW);
  digitalWrite(umidpeste, LOW);
  digitalWrite(motoras, LOW);
  digitalWrite(motoras1, LOW);  
  digitalWrite(eroarepin, LOW);

// set push buttons for menu
  pinMode(BUT1, INPUT);
  pinMode(BUT2, INPUT);
  pinMode(BUT3, INPUT);
  
  digitalWrite(BUT1, HIGH); // pull-ups on
  digitalWrite(BUT2, HIGH);
  digitalWrite(BUT3, HIGH);

  lcd.setCursor(1,0);  
  lcd.print("Incubator with");
  
  lcd.setCursor(0,1);  
  lcd.print("humidity control");

delay(3000);
lcd.clear();

  lcd.setCursor(0,0);  
  lcd.print("original sketch");
  
  lcd.setCursor(0,1);  
  lcd.print("by niq_ro  v.2.0");
delay(3000);
lcd.clear();

heat = 1;  // is 1 for heater and 0 for cooler
dry = 1;   // is 1 for dryer and 0 for wetter 

//*nu, in mare eu trebuie sa calculez puterea disp

if (EEPROM.read(200) != 199)
 // no chance
{
// just first time... after must put commnent (//)
EEPROM.write(201,1);    // tset1
EEPROM.write(202,120);  // tset2
EEPROM.write(203,5);    // dt x 10
EEPROM.write(204,45);   // hass
EEPROM.write(205,5);    // dehas
EEPROM.write(206,2);    // time in hour between spins
EEPROM.write(207,3);    // time in seconds for rotation
EEPROM.write(208,20);   // ete (error temperature) x10
EEPROM.write(209,3);    // ehas (error humidity)
EEPROM.write(210,21);   // total days of hatching
EEPROM.write(211,3);    // first day of rotating
EEPROM.write(212,18);   // last day of rotating
EEPROM.write(213,0);    // STOP = 0, START = 17
EEPROM.write(200,199);   // to nor write again
}
  
byte tset1 = EEPROM.read(201);
byte tset2 = EEPROM.read(202);
tes = 256 * tset1 + tset2;  // recover the number
tes = tes/10;
dete = EEPROM.read(203);
dete = dete/10;
hass = EEPROM.read(204);
dehas = EEPROM.read(205);
taim1 = EEPROM.read(206);
taim2 = EEPROM.read(207);  
ete = EEPROM.read(208);
ete = ete/10;
ehas = EEPROM.read(209);
tzile = EEPROM.read(210);
zi1 = EEPROM.read(211);
zi2 = EEPROM.read(212);
shtart = EEPROM.read(213);

//
if (debug)
  {
taim11 = taim1 * 60000;  // 60000 minute -> ms
  }
  else
taim11 = taim1 * 3600000;  // hour -> ms
taim22 = taim2 * 1000; // seconds -> ms

sfrotatii = millis();
}

void loop()
{
if (meniu >= 13)
{
  lcd.clear();
  meniu = 0;
}

if (meniu == 0)
{
    pushlength = pushlengthset;
    pushlength = getpushlength ();
   lcd.setCursor(8, 0);
   lcd.print(" ");
    delay (10);

    
    if (pushlength > pushlengthset)
    {
   meniu = 1;   
 
    pushlength = pushlengthset;
    delay(50);
    intraremeniu = millis();  // store time when enter in setup menu
    }
     if (pushlength < pushlengthset)
    {
   lcd.setCursor(8, 0);
   lcd.print("!");
    pushlength = pushlengthset;
    }
    
 // Reading temperature or humidity 
 has = dht.readHumidity();
  
// using DS18B20 ( http://arduinoprojects.ru/2014/08/%D0%BF%D1%80%D0%BE%D1%81%D1%82%D0%BE%D0%B9-%D1%82%D0%B5%D1%80%D0%BC%D0%BE%D1%81%D1%82%D0%B0%D1%82-%D0%BD%D0%B0-arduino-%D0%B8-%D1%86%D0%B8%D1%84%D1%80%D0%BE%D0%B2%D0%BE%D0%BC-%D1%82%D0%B5%D1%80%D0%BC/ )
sensors.requestTemperatures(); // запрос на получение температуры
float te=(sensors.getTempCByIndex(0)); 
  delay(150/ (1 << (12-resolution)));

// part from http://nicuflorica.blogspot.ro/2014/10/ceas-rtc-cu-ds1307-si-date-mediu-cu.html
 lcd.setCursor(10, 1); 
   lcd.print(te,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C");

   lcd.setCursor(10, 0);
  // lcd.print("H=");
   lcd.print(has);
   lcd.print("%RH ");

if ((te >= tes + ete) or (te <= tes - dete - ete ))
 {
eroaret = 1;
 lcd.setCursor(9, 1);
 lcd.print("*");
 } 
 else
 {
  eroaret = 0;
  lcd.setCursor(9, 1);
  lcd.print(" ");
 }
if ((has < hass - dehas - ehas) or (has > hass + ehas))
 {
 eroareh = 1;
 lcd.setCursor(9, 0);
 lcd.print("*");
 } 
else
{
  eroareh = 0;
  lcd.setCursor(9, 0);
  lcd.print(" ");
}

if ((eroaret == 1) or (eroareh == 1))
{
  eroare = 1;
}

if (eroare == 1)
{
  if (anulare == 0)
  {
  digitalWrite(eroarepin, HIGH);
  if ((digitalRead(BUT1) == LOW) or (digitalRead(BUT2) == LOW))
    {
    anulare = 1;
    delay(250);
    }
  }
  else
  {
    digitalWrite(eroarepin, LOW);
  }
}
else
{
  digitalWrite(eroarepin, LOW);
}

if ((eroaret == 0) and (eroareh == 0))
{
  eroare = 0;
  anulare = 0;
}

if (shtart == 17)  // if STAR was pushed
{
DateTime now = rtc.now();
zi = now.day(), DEC;
ora = now.hour(), DEC;
minut = now.minute(), DEC;
secunda = now.second(), DEC;

if (debug)
  {
  timp = minut;
  timp0 = tzile;
  timp1 = zi1;
  timp2 = zi2;    
  }
  else
{
  timp = (zi-1) * 1440 + ora * 60 + minut;
  timp0 = tzile * 1440;
  timp1 = zi1 * 1440;
  timp2 = zi2 * 1440;
}

dtimp = timp0 - timp;
Serial.println(dtimp);
dzi = dtimp/1440;
dtimp = dtimp%1440;
dora = dtimp/60;
dminut = dtimp%60;



  
// just if is ok
if (te > tes) 
 {
if (heat == 1) digitalWrite(temppeste, LOW); 
if (heat == 0) digitalWrite(temppeste, HIGH); 
 } 
if (tes - dete > te)
  {
if (heat == 0) digitalWrite(temppeste, LOW); 
if (heat == 1) digitalWrite(temppeste, HIGH); 
 } 
 if (has > hass) 
 {
if (dry == 1) digitalWrite(umidpeste, HIGH); 
if (dry == 0) digitalWrite(umidpeste, LOW); 
 } 
 if (has < hass - dehas) 
 {
if (dry == 0) digitalWrite(umidpeste, HIGH); 
if (dry == 1) digitalWrite(umidpeste, LOW); 
 }

 /*
 lcd.print("rem:");
 if ((ida >=0) && (ida <10)) lcd.print(" ");
 lcd.print(ida);
 lcd.print("d");
 */

 if (timp <= timp0)
 {
  lcd.setCursor(0, 0);
  if (dzi < 10) lcd.print(" ");  
  lcd.print(dzi); 
  lcd.print("d");
  if (dora < 10) lcd.print("0");  
  lcd.print(dora); 
  lcd.print(":");  
  if (dminut < 10) lcd.print("0");  
  lcd.print(dminut); 
//  lcd.print("m ");  
 }
else
{
  lcd.setCursor(0, 0);
  lcd.print("THE END ");  
}

/*  // original
// spins part (turning eggs)
while (((millis() - sfrotatii) > taim11) && (rotit == 0))
{
  lcd.setCursor(0, 1);
  lcd.print(" Turning EGGs!  ");
  digitalWrite(motoras, HIGH); 
  delay(taim22);
  digitalWrite(motoras, LOW); 
  rotit = 1;
}

if (rotit == 1)
{
  sfrotatii = millis();
  rotit = 0;
  lcd.clear();
}
*/  
if ((timp > timp1) and (timp < timp2))
{ 
// part for day remaining
  taim=millis();                              //  Get time in milliseconds since tunit turn on
  s1=taim/1000;                               //  Convert time to seconds, minutes, hours, days
  m1=s1/60;
  h1=m1/60;
  d1=h1/24;  
  id=int(d1);                                 //  Strip out remainder to leave Days:Hours:Minutes:Seconds
  ih=int((d1-int(d1))*24);
  im=int((h1-int(h1))*60);
  is=int((m1-int(m1))*60);
// Calculate approximate days till hatch (assume 21 days to hatch)
  ida=21-id;

 tpintors =  - millis() + taim11 + sfrotatii;  // time to rotatitng again
 tpintors = tpintors /1000; // ms -> s
 ora1 = tpintors/3600;   
 rest1 = tpintors - 3600*ora1;
 minut1 = rest1/60;
 secunda1 = rest1 - 60*minut1;

 lcd.setCursor(0, 1);
 if (ora1 <10) lcd.print(" ");
 lcd.print(ora1);
 lcd.print(":");
 if (minut1 <10) lcd.print("0");
 lcd.print(minut1);
 lcd.print(":");
 if (secunda1 <10) lcd.print("0");
 lcd.print(secunda1);
 lcd.print(" ");
 lcd.setCursor(0, 0);
 
// spins part (turning eggs)
while (((millis() - sfrotatii) > taim11) && (rotit == 0))
{
  Serial.print(motordr);
  lcd.setCursor(0, 1);
  lcd.print(" Turning EGGs!  ");
  if (motordr%2 == 0) {
  digitalWrite(motoras, HIGH); 
  digitalWrite(motoras1, LOW);
  } 
   if (motordr%2 == 1) {
  digitalWrite(motoras, LOW); 
  digitalWrite(motoras1, HIGH);
  } 
  delay(taim22);
  digitalWrite(motoras, LOW); 
  digitalWrite(motoras1, LOW); 
  rotit = 1;
  motordr++;
  /*
   if (motordr == 1)  motordr = 0 ;
   if (motordr == 0)  motordr = 1;  
   */
   Serial.print("/");
   Serial.println(motordr);
   
}

if (rotit == 1)
{
  sfrotatii = millis();
  rotit = 0;
  
  lcd.clear();
}
} // end part with good time for rotating
else
{
 lcd.setCursor(0, 1);
 lcd.print("         ");
}

} // end of START

} // last line for MENIU = 0


if (meniu == 1) {
  while (meniu == 1) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
//   teset(tset);
   lcd.setCursor(0, 0);
   lcd.print("Temperature SET:");
   lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");


   if (digitalRead(BUT1) == LOW)
  { tes = tes - 0.1; 
  delay(250);
  }
   if (digitalRead(BUT2) == LOW)
  { tes = tes + 0.1;
  delay(250);
  }

int tes2 = tes*10;  
byte tset1 = tes2 / 256;
byte tset2 = tes2 - tset1 * 256;
  
  if (digitalRead(BUT3) == LOW) 
  {
   EEPROM.write(201, tset1);  // partea intreaga
   EEPROM.write(202, tset2);   // rest
   meniu = 2;
   delay(250);
   lcd.clear();
  }
//  delay(15);
     }
   delay (100);
}   // end loop for MENIU = 1


if (meniu == 2) {
   while (meniu ==2) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("hyst.temp. SET:");
   lcd.setCursor(0, 1);
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

    if (digitalRead(BUT1) == LOW) 
   { dete = dete - 0.1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { dete = dete + 0.1;
   delay(250);
  }
//   delay(15);
if (dete < 0.1) dete = 0.1;
if (dete > 1.0) dete = 1.0;

   if (digitalRead(BUT3) == LOW) 
   {
   EEPROM.write(203,dete*10);      
   meniu = 3;
   delay(250);
   lcd.clear();
  }
    }
}  // end loop for MENIU = 2

if (meniu == 3) {
  while (meniu == 3) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("Humidity SET:");
   lcd.setCursor(0, 1);
   lcd.print(hass,1);

   lcd.print("%RH  ");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);

   lcd.print("%RH");


   if (digitalRead(BUT1) == LOW)
  { hass = hass - 1; 
  delay(250);
  }
   if (digitalRead(BUT2) == LOW)
  { hass = hass + 1;
  delay(250);
  }
   
  if (digitalRead(BUT3) == LOW) 
  { 
   EEPROM.write(204,hass);      
   meniu = 4;
  delay(250);
  lcd.clear();
  }
//  delay(15);
     }
   delay (250);
}  // end loop for MENIU =3


if (meniu == 4) {
   while (meniu ==4) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("hyst.humid. SET:");
   lcd.setCursor(0, 1);
   lcd.print(hass);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH");

 
   if (digitalRead(BUT1) == LOW) 
   { dehas = dehas - 1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { dehas = dehas + 1;
   delay(250);
  }
//   delay(15);
if (dehas < 1) dehas = 1;
  
  if (digitalRead(BUT3) == LOW) 
   {
    EEPROM.write(205,dehas);
    meniu = 5;
   delay(250);
   lcd.clear();
  }
    }
}  // end loop for MENIU = 4


if (meniu == 5) {
   while (meniu ==5) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("Time between");
   lcd.setCursor(0, 1);
   lcd.print("spins : ");

   if (digitalRead(BUT1) == LOW) 
   { taim1 = taim1 - 1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { taim1 = taim1 + 1;
   delay(250);
  }

  if (taim1 < 1) taim1 = 24;
  if (taim1 > 24) taim1 = 1;

lcd.setCursor(9, 1);
lcd.print(taim1);
lcd.print("h ");
 
   if (digitalRead(BUT3) == LOW) 
   { 
   EEPROM.write(206,taim1);     
   delay(250);
   lcd.clear();
if (debug)
  {
  taim11 = taim1 * 60000;  // minute -> ms
  }
  else
taim11 = taim1 * 3600000;  // hour -> ms
   meniu = 6;
  }
  
  }
}   // end loop for end MENIU = 5

if (meniu == 6) {
   while (meniu ==6) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("Time ON motor: ");
   
   if (digitalRead(BUT1) == LOW) 
   { taim2 = taim2 - 1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { taim2 = taim2 + 1;
   delay(250);
  }

  if (taim2 < 3) taim2 = 3;
  if (taim2 > 10) taim2 = 10;

lcd.setCursor(12, 1);
lcd.print(taim2);
lcd.print("s ");

   if (digitalRead(BUT3) == LOW) 
   { 
   EEPROM.write(207, taim2);    
   delay(250);
   lcd.clear();
taim22 = taim2 * 1000; // seconds -> ms
   meniu = 7;
  }
  }  
}   // end loop for end MENIU = 6  


if (meniu == 7) {
   while (meniu ==7) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("ERR temperature:");

   lcd.setCursor(5, 1);
   lcd.print("et=+/-");
   lcd.print(ete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

    if (digitalRead(BUT1) == LOW) 
   { ete = ete - 0.1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { ete = ete + 0.1;
   delay(250);
  }

if (ete < 0.1) ete = 0.1;
if (ete > 3.0) ete =3.0;

   if (digitalRead(BUT3) == LOW) 
   {
   EEPROM.write(208,ete*10);      
   meniu = 8;
   delay(250);
   lcd.clear();
  }
    }
}  // end loop for MENIU = 7

if (meniu == 8) {
   while (meniu ==8) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("ERR humidity:");

   lcd.setCursor(3, 1);
   lcd.print("eh=+/-");
   lcd.print(ehas);
 //  lcd.write(byte(0));
   lcd.print("%RH");

   if (digitalRead(BUT1) == LOW) 
   { ehas = ehas - 1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { ehas = ehas + 1;
   delay(250);
  }
if (ehas < 1) ehas = 1;
if (ehas > 10) ehas = 10;  
  if (digitalRead(BUT3) == LOW) 
   {
    EEPROM.write(209,ehas);
   delay(250);
   lcd.clear();
    meniu = 9;
  }
    }
}  // end loop for MENIU = 8

if (meniu == 9) {
   while (meniu == 9) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("Total days :");

   lcd.setCursor(3, 1);
   lcd.print(tzile);
   lcd.print("    ");


   if (digitalRead(BUT1) == LOW) 
   { tzile = tzile - 1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { tzile = tzile + 1;
   delay(250);
  }

if (tzile < 8) tzile = 8; // 16
if (tzile > 52) tzile = 52;
  
  if (digitalRead(BUT3) == LOW) 
   {
    EEPROM.write(210,tzile);
   delay(250);
   lcd.clear();
    meniu = 10;
  }
    }
}  // end loop for MENIU = 9


if (meniu == 10) {
   while (meniu == 10) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("Spin's first day");

   lcd.setCursor(3, 1);
   lcd.print(zi1);
   lcd.print(" of ");
   lcd.print(tzile);
   lcd.print("  ");

   if (digitalRead(BUT1) == LOW) 
   { zi1 = zi1 - 1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { zi1 = zi1 + 1;
   delay(250);
  }

if (zi1 < 2) zi1 = 2;
if (zi1 > 4) zi1 = 4;
  
  if (digitalRead(BUT3) == LOW) 
   {
    EEPROM.write(211,zi1);
   delay(250);
   lcd.clear();
    meniu = 11;
  }
    }
}  // end loop for MENIU = 10

if (meniu == 11) {
   while (meniu == 11) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   lcd.print("Spin's last day");

   lcd.setCursor(3, 1);
   lcd.print(zi2);
   lcd.print(" of ");
   lcd.print(tzile);
   lcd.print("  ");

   if (digitalRead(BUT1) == LOW) 
   { zi2 = zi2 - 1;
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   { zi2 = zi2 + 1;
   delay(250);
  }

if (zi2 < tzile - 4) zi2 = tzile - 4;
if (zi2 > tzile - 2) zi2 = tzile - 2;
  
  if (digitalRead(BUT3) == LOW) 
   {
    EEPROM.write(212,zi2);
   delay(250);
   lcd.clear();
    meniu = 12;
  }
    }
}  // end loop for MENIU = 12

if (meniu == 12) {
   while (meniu == 12) {
    if (millis() - intraremeniu > iesiremeniu) meniu = 13;
   lcd.setCursor(0, 0);
   if (shtart == 0)
   lcd.print("Hatching is off!");
   if (shtart == 17)
   lcd.print("Hatching is ON !");
   lcd.setCursor(0, 1);
   lcd.print("STOP(-) START(+)");

   if (digitalRead(BUT1) == LOW) 
   {
    shtart = 0;
    //lcd.print("STOP");
   delay(250);
  }
   if (digitalRead(BUT2) == LOW) 
   {
    shtart = 17;
   //lcd.print("START");
   delay(250);
  }
  
  if (digitalRead(BUT3) == LOW) 
   {
   EEPROM.write(213,shtart);
   if (shtart == 17) rtc.adjust(DateTime(1973, 1, 1, 0, 0, 0));
   delay(250);
   lcd.clear();
   meniu = 13;
  }
    }
}  // end loop for MENIU = 12





}   // last line in main loop





// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(BUT3);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
/*
     if (millis() - intraremeniu < pushlengthset)
    {
   lcd.setCursor(8, 0);
   lcd.print("?");
    }  
*/     
       return pushlength;
}

/*
 // subroutine to return the length of the button push.

*/
