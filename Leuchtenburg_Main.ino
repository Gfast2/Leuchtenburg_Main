/* Code derived from "Leuchtenburg_2015_04_24". 
 * In this version we write new sound control code for the new "WAVE Trigger" board. Besides the Old sound effect. We add 20 text sound tracks to give user better acustic experience on this maschine.
 *
 * Written: Tom Hanke & Su Gao
 * Last Edite: 13.Mai 2015
 */

#include <SoftwareSerial.h>
SoftwareSerial LEDSerial(20, 21);

//Einstellbare Werte
const long Waagezeit_Konstante = 120000; //Wartezeit bei fehlendem Statuswechsel bis Reset
const long RelativMaxWeg = 12000;   //Maximale Auslenkung Waagen (x2 weil 2 gestaffelte Waagen)
const long WegMinimum = 3000;       //Minimale Auslenkung Waagen
const int G_SchwellwertMin = 3;     //Gewichtsänderungen ab Wert * 10g werden registriert
const int Minimalgewicht = 10;      //wenn Gewicht < Minimalgewicht dann keine Lösung möglich  
const int G_Max = 250;            //Maximalgewicht (Wert*10g), für Skalierung Beschleunigung, V_Max und Fehlererkennung 
const int G_Max_Calib = 50;
const int G_MaxChange = 100;      //Bei spontaner Gewichtsänderung größer Wert stoppen die Motoren (Fehlererkennung) 
const long FaktorGewicht = 200;   //Umrechnung Gewichtsdifferenz auf Weg (GetStatus) (Bei 500g Gewichtsdifferenz maximale Abstände zwischen Schalen)
const float BremsFaktor = 4.;     //Verhältnis Beschleunigung zu Bremse
const float a_Min = 1.;           //Minimalbeschleunigung in Hz/ms; war 2.
const float a_Max = 5.;           //Maximalbeschleunigung in Hz/ms; 
const int Schwellwert = 200;      //zulässige Differenz zur Zielposition
const int WrongFadeOut = 5000;     //Zeit in ms wie lange Falscher Ton bleibt
const long MagnetMaxTime = 12000;
//const long BremsWegMax = 10000; //optional falls reale Geschwindigkeit einbezogen wird

long MagnetOnTime;

boolean ZuSchwer[5] = {false, false, false, false, false};
int FirstTimeError = 0;
boolean CalibError[5] = {false, false, false, false, false};
long CalibErrorPause[5];
int G_MaxFlag = 0;  //Set when G > G_Max

int FirstStart = 1;

//Pin Definition

int chipDriver = 2;                     //RS485 Treiber für Gewichtstransmitter (HalbDuplex)
int EndSchalterPin[5]={0,40,42,52,50};  //Induktive Endschalter
int MagnetPin[5]={0,51,53,41,43};       //Zugmagnete
int NotAusPin = 3;                      //NotTaste vom Kontrollraum aus

//Ende Pin Definition

int NotAusStatus = 0;
boolean NotAusPressed = false;

int StartWert = 1;
int DoneFlag = 0;

boolean GewichtChange4 = false;

int G_Schwellwert = G_SchwellwertMin;      //Gewichtsänderungen ab Wert * 10g werden registriert
long Waagezeit_inaktiv;   


int Durchlauf[5];    //Für Pendelsimulation Anzahl der Durchläufe zur Zielposition

int MotorOben = 1;  //aktuell höchster Motor zur Übergabe der Fahrtcharakteristika an Motor4  

float a_Ref;
float a_Aktuell[5];
long b_Value[5];
long a_Value[5];

const long v_Max = 40000; //war 25000

int EndSchalter[5];

int KippStatus[5];

long Position[5]={0,0,0,0,0};
long PositionAlt[5];
long PositionSoll[5];

//-----------------Für Berechnung aktuelle Geschwindigkeit und Bremsweg
/*
long v_Ref;
long v_Aktuell[5];
const long v_Min = 2000;
long Bremsweg[5];
float Speed[5];
long TimeAlt[5];
long TimeNeu[5];
*/
//-----------------

int Richtung[5];

boolean M_Done[5]={true, true, true, true, true};

//-----

long Gewicht[5];
long GewichtAlt[5];

int GewichtStatus[5];    //Status 0: Normal, Status 1: ausgelesener Wert rubbish, Status 2: Gewicht zu hoch, Status 3: anything else 

int G_Change = 0;

int WaageBeladen = 0; //Anzahl der Waagen mit Gewicht

float FaktorRichtig;  //Faktor wir Nahe Ergebnis erreicht ist (1 = Perfekt);
int ZustandFaktor = 0; //Faktor >= 2. Zustand 0, Faktor >= 1. Zustand 1, Faktor < 1. Zustand 2;  

int GewichtSumme; //Summe aller Schalengewichte;
int Volume4;

long TimeWrongSound;

boolean FaktorWarRichtig = false; 
boolean TimeWrongGestartet = false;

int StatusNeu;
int StatusAlt; 
boolean StatusChange = false;
//-----

long drehwert = 0;

long kippPosition = -45000;
long kippPosUnten = -80000;

long ErrorDist = -30000;    //wenn Gewicht auf Schaale 4 dann fährt ein Motor um ErrorDistanz nach oben

long PosOben = -35000;
long PosMitteOben = -45000;
long PosMitte = -55000;
long PosMitteUnten = -65000;
long PosUnten = -75000;

long AnschlagOben = -10000;    //Schutzwert falls Waageschale zu hoch oder zu tief fährt
long AnschlagUnten = -93000;

long WegW1, WegW2;

long StartPosition[5]={0, PosMitteOben, PosMitteUnten, PosMitteUnten, PosMitte};


//------------------------------------------------------------------------------------
/*
 _______  _______ _________          _______ 
(  ____ \(  ____ \\__   __/|\     /|(  ____ )
| (    \/| (    \/   ) (   | )   ( || (    )|
| (_____ | (__       | |   | |   | || (____)|
(_____  )|  __)      | |   | |   | ||  _____)
      ) || (         | |   | |   | || (      
/\____) || (____/\   | |   | (___) || )      
\_______)(_______/   )_(   (_______)|/       
*/                                       

void setup()
{
  Serial3.begin(115200); //Gewichtstransmitter
  Serial2.begin(115200); //Motoren
  Serial.begin(115200); //Rechner
  //Serial1.begin(9600); //2CH Musik board
  Serial1.begin(57600); //WAVE Trigger board from Sparkfun
  
  LEDSerial.begin(38400);  //Softwareserial für LEDs und Musikshield

  
  pinMode(chipDriver,OUTPUT);
  digitalWrite(chipDriver, LOW);    //LOW setzt RS485 Chip auf Listen 

  pinMode(EndSchalterPin[1], INPUT);
  pinMode(MagnetPin[1], OUTPUT);
  pinMode(EndSchalterPin[2], INPUT);
  pinMode(MagnetPin[2], OUTPUT);
  pinMode(EndSchalterPin[3], INPUT);
  pinMode(MagnetPin[3], OUTPUT);
  pinMode(EndSchalterPin[4], INPUT);
  pinMode(MagnetPin[4], OUTPUT);
  
  pinMode(NotAusPin, INPUT);

  delay(1000);
  
      MotorHardStop(1);
      MotorHardStop(2);
      MotorHardStop(3);
      MotorHardStop(4);   

  delay(5000);
//LICHT//-------------------------------------------------------------------  
  LEDSerial.print("kerze;");       //"kerze" bedeutet flackern
  delay(50);  
//SOUND//-------------------------------------------------------------------  
  // Start these tracks to play.
  for(int i = 31; i<36; i++)
    play(i);
  // Set these five channel loop.
  for(int i = 31; i<36; i++)
    trackLoop(i,1); //turn on track loop
    
  //LEDSerial.print("sd A90 B90 C90 D90 E90;"); // fade out all ambient sound
  /*
  volumnTrack(31,-70);  // A
  volumnTrack(32,-70);  // B
  volumnTrack(33,-70);  // C
  volumnTrack(34,-70);  // D
  volumnTrack(35,-70);  // E
  */
  soundAmb(1); //pack all hard coded command together.
  delay(50);
//SOUND//-------------------------------------------------------------------

    Serial.println("setup finished 2"); 
   
}





//----------------------------------------------------------------------------------------------------------------------------------
/*
 _        _______  _______  _______ 
( \      (  ___  )(  ___  )(  ____ )
| (      | (   ) || (   ) || (    )|
| |      | |   | || |   | || (____)|
| |      | |   | || |   | ||  _____)
| |      | |   | || |   | || (      
| (____/\| (___) || (___) || )      
(_______/(_______)(_______)|/  
*/

void loop()
{
  
NotAusStatus = digitalRead(NotAusPin);
    
if ((NotAusStatus == 1)||(NotAusPressed == true)){
   NotAusHandling();
}
 
else {        //Wenn kein Notausgedrückt ist NormalSchleife 


//----------Check ob Neukalibirierung erforderlich (Berührung Endschalter oder lange Inaktiv)------------

            //Endschalter einlesen, wenn Kontakt dann Neukalibrierung  


  
            for (int i=1;i<=4;i++){                            
              EndSchalter[i] = digitalRead(EndSchalterPin[i]);
            }
              
            if ((EndSchalter[1] == 0)||(EndSchalter[2] == 0)||(EndSchalter[3] == 0)||(EndSchalter[4] == 0)) {  //wenn Endschalter Kontakt Neukalibrierung 
             StartWert = 1; 
            }
          
            if ((millis()-Waagezeit_inaktiv) > Waagezeit_Konstante){
             StartWert = 1; 

            }
  
//--------------------------------------------------------------------------------------------------------------
  
if (StartWert == 1){        //Beim ersten Durchlauf kein G_Max Handling
      if (FirstStart != 1){
          G_MaxHandling();  
          }
          calibration();
          if (FirstStart == 1){
          FirstStart = 0;
          }
      }
      
    
else {
  
 //------------Gewicht einlesen und Gewichtsänderung feststellen-------------------------------------------------
  StatusChange = false; 
  
  G_Change = 0;          //Gewichtswechsel-Flag auf NULL setzen
  WaageBeladen = 0;
  for (int j=1;j<=4;j++){
    weightRead(j);//Gewicht einlesen
    delay(5);//Pause zum nächsten Transmitter
    
    if (Gewicht[j] < 0){Gewicht[j] = 0;}
    
    //Serial.print(String(j) + ": ");
    //Serial.println(Gewicht[j]);
    
    //G_Schwellwert = G_SchwellwertMin + (Gewicht[j]/20.);
  
    if (Gewicht[j] > G_Max){
        ZuSchwer[j] = true;
    }
    else ZuSchwer[j] = false;
  
    if (Gewicht[j]>G_SchwellwertMin){  //Music
        WaageBeladen += 1;             //Wenn Gewicht auf Waage Variable + 1;
       }                               //Music
   }
   
  if ((ZuSchwer[1] == true)||(ZuSchwer[2] == true)||(ZuSchwer[3] == true)||(ZuSchwer[4] == true)){
     Errorhandling(); 
  }

  else{
     if (FirstTimeError == 2){
        FirstTimeError = 0;
//LICHT//-------------------------------------------------------------------
        LEDSerial.print("kerze;"); 
     }
     
// Soundkanal4 Steuerung (Nahe an Loesung)
   GewichtSumme = Gewicht[1] + Gewicht[2] + Gewicht[3];
   
   G_Schwellwert = G_SchwellwertMin + (GewichtSumme /20.);
   
   if ((GewichtSumme > 35) && ((Gewicht[1] != 0) && (Gewicht[2] != 0) && (Gewicht[3] != 0) && (Gewicht[4] < G_SchwellwertMin))){ 
   FaktorRichtig = (abs(((GewichtSumme / 2.) / Gewicht[1])-1) + abs(((GewichtSumme / 4.) / Gewicht[2])-1) + abs(((GewichtSumme / 4.) / Gewicht[3])-1));
   //FaktorRichtig = FaktorRichtig / 3.;
   }
   else {
     FaktorRichtig = 2.;
     //Channel4 leise
     }
     
   //Serial.print("Faktor: ");
   //Serial.print(FaktorRichtig);
  
   if (((FaktorRichtig <= 0.6)&&(GewichtSumme > 10)) && (Gewicht[4] < G_SchwellwertMin)){  //Fast Richtig
      if (FaktorWarRichtig == false){                   
      //LEDSerial.print("sd D" + String(Volume4) + ";");
//SOUND//------------------------------------------------------------------- 
      //LEDSerial.print("sd D10 E90;"); //turn on channel four, turn down channel five.
      /*
      volumnTrack(34,-5);  // D
      volumnTrack(35,-70); // E
      */
      soundAmb(2);
      FaktorWarRichtig = true;
      TimeWrongGestartet = false;
      //Serial.println("Faktor kleiner 1!");
      }
   }

   else if (FaktorWarRichtig == true){   //nach Fast Richtig wieder falsch
      FaktorWarRichtig = false; 
     //LEDSerial.print("sd D" + String(Volume4) + ";");
//SOUND//-------------------------------------------------------------------
      //LEDSerial.print("sd D90 E10;");
      /*
      volumnTrack(34,-70); // D
      volumnTrack(35,-5);  // E
      */
      soundAmb(3);
      TimeWrongSound = millis();
      TimeWrongGestartet = true;
      //Serial.println("Faktor raus aus 1!");
   }

   if (TimeWrongGestartet == true){ 
      if ((millis()-TimeWrongSound) > WrongFadeOut){ 
//SOUND//-------------------------------------------------------------------
      //LEDSerial.print("sd D90 E90;");
      /*
      volumnTrack(34,-70);  // D
      volumnTrack(35,-70);  // E
      */
      soundAmb(4);
      TimeWrongGestartet = false;
      //Serial.println("FadeOutOver!");
      }
   }   
   
   //Serial.print(", Zustand: ");
   //Serial.println(ZustandFaktor);
   
   //Serial.print("Faktor: ");
   //Serial.println(Volume4);
   
    
    
    
    
    for (int j=1;j<=4;j++){   
    
          if (abs(GewichtAlt[j]-Gewicht[j])>G_Schwellwert){
            G_Change = 1;
            GewichtAlt[j] = Gewicht[j];  

            if (abs(GewichtAlt[j]-Gewicht[j])>(G_MaxChange)){
      
                G_Change = 0;
                MotorHardStop(1);
                MotorHardStop(2);
                MotorHardStop(3);
                MotorHardStop(4);
                }            
            }     
    }
   
//-----------Ende Gewicht einlesen---------------------------------------------------
 
 
//-----------Begin Statusabfrage (neue SollPositionen für Motoren)------------------- 

      if (((GewichtSumme + Gewicht[4] <= G_SchwellwertMin) && (G_Change == 0)) && (StatusNeu != 13)) G_Change = 1; 
      
  
      if ((G_Change == 1)&&(DoneFlag != 1)){      //Wenn Gewichtsänderung dann Motor-Werte aktualisieren 
          
//Music     StartChannelsWaagenSound; //Kanal 1 - 3 anschalten
//Music     Abhängig vom Faktor Kanal 4 zuschalten
          
          if (WaageBeladen >= 3){           
//SOUND//-------------------------------------------------------------------
            //LEDSerial.print("sd A0 B0 C0;");
            /*
            volumnTrack(31,-5);  // A
            volumnTrack(32,-5);  // B
            volumnTrack(33,-5);  // C
            */
            soundAmb(5);
          }
          else if (WaageBeladen >= 2){           
//SOUND//-------------------------------------------------------------------
//              LEDSerial.print("sd A0 B0 C90;");
              /*
              volumnTrack(31,-5);  // A
              volumnTrack(32,-5);  // B
              volumnTrack(33,-70); // C
              */
              soundAmb(6);
          }
          else if (WaageBeladen >= 1){     
//SOUND//-------------------------------------------------------------------      
              //LEDSerial.print("sd A0 B90 C90;");
              /*
              volumnTrack(31,-5);  // A
              volumnTrack(32,-70);  // B
              volumnTrack(33,-70);  // C
              */
              soundAmb(7);
          }
          if (WaageBeladen == 0){           
//SOUND//-------------------------------------------------------------------
              //LEDSerial.print("sd A90 B90 C90;");
              /*
              volumnTrack(31,-70);  // A
              volumnTrack(32,-70);  // B
              volumnTrack(33,-70);  // C
              */
              soundAmb(8);
              //SoundZustand(0);    //keine Waage in Benutzung --> Standby Text   
          }
          

          Waagezeit_inaktiv = millis();           //Falls StatusÄnderung Reset Waagezeit_inaktiv (für Restart nach bestimmter Inaktivzeit)     
          StatusNeu = GetStatus(Gewicht[1], Gewicht[2], Gewicht[3], Gewicht[4]);    //Alle aktuellen Gewichtswerte an Funktion "Getstatus" übergeben
                                                                                    //Funktion gibt Wert zwischen 1 und 15 zurück (SollPositionen)         
          if (StatusNeu == 15) StatusNeu = StatusAlt;                           //Wenn Minimalgewicht nicht erreicht Status belassen
       
          SetNewStatus(StatusNeu, Gewicht[4]);                                  //Aus Statusnummer jeweilige SollPositionen setzen
          StatusAlt = StatusNeu;
          StatusChange = true;
      
      
          for (int i=1; i<=4;i++){
          Durchlauf[i] = 0;        //Für Nachschwingdurchgänge beim Bewegen
          }

      }
    
   //-------------- Sonderfall Rätsel gelöst  
      
      else if ((StatusNeu == 14)&&(DoneFlag == 0)){              //DONE!!!
	DoneFlag = 1;
        FaktorWarRichtig = false;     
//SOUND//-------------------------------------------------------------------
        //LEDSerial.print("sd A90 B90 C90 D90 E90;");
        /*
        volumnTrack(31,-70);  // A
        volumnTrack(32,-70);  // B
        volumnTrack(33,-70);  // C
        volumnTrack(34,-70);  // D
        volumnTrack(35,-70);  // E
        */
        soundAmb(1);

//SOUND//-------------------------------------------------------------------
        //Serial1.write((byte)0x31); // Direct-Song-Play
        stop(30);
        play(36); // only when the finish sound start to play, stop this sound.
        volumnTrack(36,-5);
        trackLoop(36,1); //loop this sound
        delay(50);
        //Serial1.write((byte)0x0D); // Start to play
//LICHT//-------------------------------------------------------------------
        LEDSerial.print("blink;");         
      }
       
//Serial.println(StatusNeu);
     
      if (StatusNeu == 13){
        Waagezeit_inaktiv = millis();           //Falls StatusÄnderung Reset Waagezeit_inaktiv (für Restart nach bestimmter Inaktivzeit)

      }
            
      
   //-------------- Ende Sonderfall
   
//------------------Ende StatusZuweisung--------------------------------------------------
  

//------------------Begin Motorfahrt / Fahrtkontrolle-------------------------------------

  if ((StatusChange == true) || (M_Done[1] == false) || (M_Done[2] == false) || (M_Done[3] == false) || (M_Done[4] == false)){
  MotorFahrt();
  }
  
  if ((DoneFlag == 1) && (M_Done[1] == true) && (M_Done[2] == true) && (M_Done[3] == true) && (M_Done[4] == true)){
    delay(7000);
    LEDSerial.print("kerze;");       
//SOUND//-------------------------------------------------------------------
    //Serial1.write((byte)0x30);
    stop(36);
    play(30); // only when the finish sound start to play, stop this sound.
    volumnTrack(30,-5);
    trackLoop(30,1); //loop this sound
    
    DoneFlag = 0;
    calibration();
    
    delay(1000);
  }
  
  
  if (G_Change == 1) UpdateMotorCharacter();    //Bei Gewichtsänderung MotorWerte (Beschleunigung, MaximalSpeed, Bremsbeschleunigung aktualisieren)
  
  if ((M_Done[1] == true) && (M_Done[2] == true) && (M_Done[3] == true) && (M_Done[4] == true)){

      //Music  StopChannelsWaagenSound; //Kanal 1 - 3 ausschalten
//SOUND//-------------------------------------------------------------------
      //LEDSerial.print("sd A90 B90 C90;");
      /*
      volumnTrack(31,-70);  // A
      volumnTrack(32,-70);  // B
      volumnTrack(33,-70);  // C
      */
      soundAmb(8);
  
  }

  }//--------------------Ende Schleife wenn Maximalgewicht nicht ueberschritten

}//----------------------Ende Normalschleife (falls kein Endschalter gedrückt) 

}//----------------------Ende NotAus Else 

sound(StatusNeu); //I'm not sure if the statusNeu always be updated in this position.
   
} //---------------------End of loop
















