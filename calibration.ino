void calibration() {
  Serial.println("Enter calib");
  //LICHT//-------------------------------------------------------------------
  LEDSerial.print("kerze;");       //"kerze" bedeutet flackern
  //LEDSerial.print("play 1;");
  //SOUND//---------------------------------------------------------------
  //Serial1.write((byte)0x30); //direct play the first song
  //Serial1.write((byte)0x0D); //start


  //This part set all channel to 0 with default blendspeed that used in Musik & sound control board
  //LEDSerial.print("sd A0 B0 C0 D0 E0 F0 G0 H0;");
  //delay(5000);
  //LEDSerial.print("sd a15 b15 c15 d15 e15 f15 g15 h15;"); //set new default blend speed
  //delay(5000);
  //finish reset blendspeed & blendspeed reset to 15 for each channel.


  //SOUND//---------------------------------------------------------------
  //LEDSerial.print("sd a10 b10 c10 d5 e5 f10;"); //sound blendeing speed control, don't need it anymore.
  delay(100);
  //SOUND//---------------------------------------------------------------
  //LEDSerial.print("sd A90 B90 C90 D90 E90 F90;");
  /*
  volumnTrack(31,-70);  // A
  volumnTrack(32,-70);  // B
  volumnTrack(33,-70);  // C
  volumnTrack(34,-70);  // D
  volumnTrack(35,-70);  // E
  delay(50);
  */
  Waagezeit_inaktiv = millis();


  //LICHT//-------------------------------------------------------------------
  LEDSerial.print("lt E0;");    //alle Lichter ausschalter
  delay(1000);
  //LEDSerial.print("stt A B C D;");

  ZustandFaktor = 0;

  SetMotorStartValues();
  soundFlagesReset(); //reset sound routine initial value


  //--------NOTAUSROUTINE START
  NotAusStatus = digitalRead(NotAusPin);

  if ((NotAusStatus == 1) || (NotAusPressed == true)) {
    NotAusHandling();
  }
  //--------NOTAUSROUTINE START

  for (int i = 1; i <= 4; i++) {

    EndSchalter[i] = digitalRead(EndSchalterPin[i]);
    GetPosition(i);

    //Addon NotAus Start
    if ((EndSchalter[i] == 1) && (NotAusPressed == false) && (G_MaxFlag == 0)) {
      MotorStart(i);
    }
    //Addon NotAus Ende
  }

  while (((EndSchalter[1] == 1) || (EndSchalter[2] == 1) || (EndSchalter[3] == 1) || (EndSchalter[4] == 1)) && (NotAusPressed == false) && (G_MaxFlag == 0)) {

    if (FirstStart != 1) {
      G_MaxHandling();
    }

    for (int i = 1; i <= 4; i++) {
      EndSchalter[i] = digitalRead(EndSchalterPin[i]);
      if (EndSchalter[i] == 0) {
        MotorHardStop(i);
      }

      //else CheckForError(i);

    }

    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);

    if ((NotAusStatus == 1) || (NotAusPressed == true)) {
      NotAusHandling();
    }
    //--------NOTAUSROUTINE START

  }


  // -------------------------------------------------------------------------------------------------------------------------------
  // Motoren jetzt ganz oben
  // -------------------------------------------------------------------------------------------------------------------------------

  Serial2.println("#*o7000"); //Maximalfrequenz Seite50
  delay(10);
  Serial2.println("#*D0"); //Drehüberwachung Reset
  delay(10);

  if ((NotAusPressed == false) && (G_MaxFlag == 0)) { //Zusatz Notaus

    for (int i = 1; i <= 4; i++) {
      SetMotorRichtung(i, 0);
      KippStatus[i] == 0;
      MotorStart(i);
    }

  }   //Zusatz Notaus


  //-->     Serial.println("Motoren jetzt los nach unten");
  // -------------------------------------------------------------------------------------------------------------------------------
  // Motoren jetzt ganz unten und ausgeleert
  // -------------------------------------------------------------------------------------------------------------------------------

  while (((Position[1] > kippPosUnten) || (Position[2] > kippPosUnten) || (Position[3] > kippPosUnten) || (Position[4] > kippPosUnten)) && (NotAusPressed == false) && (G_MaxFlag == 0))
  {
    //-->      Serial.println(Position[1]);

    if (FirstStart != 1) {
      G_MaxHandling();
    }


    for (int i = 1; i <= 4; i++) {
      GetPosition(i);
      delay(10);
      if ((Position[i] <= kippPosition) && (KippStatus[i] == 0)) {
        KippStatus[i] = 1;
        digitalWrite(MagnetPin[i], HIGH);
      }

      if (Position[i] <= kippPosUnten) {
        MotorHardStop(i);
      }
    }
    MagnetOnTime = millis();

    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);

    if ((NotAusStatus == 1) || (G_MaxFlag == 1)) {
      NotAusHandling();
      for (int k = 1; k <= 4; k++) {
        digitalWrite(MagnetPin[k], LOW);
        KippStatus[k] = 0;

      }
    }
    //--------NOTAUSROUTINE START

  }

  //----Eingeschoben ZeroSetting


  if ((NotAusPressed == false) && (G_MaxFlag == 0)) { //Zusatz Notaus
    delay(1000);
    for (int i = 1; i <= 4; i++) {
      delay(10);
      zero(i);
      //delay(10);
      while (Serial3.available() > 0) {
        Serial3.read(); //clear the gewicht transmitter port
      }
    }
  }

  //----Ende Einschub ZeroSetting


  //-->     Serial.println("UNTEN");

  //GetPosition(i);
  //delay(10);

  if ((NotAusPressed == false) && (G_MaxFlag == 0)) { //Zusatz Notaus

    for (int i = 1; i <= 4; i++) {
      SetMotorRichtung(i, 1);
      MotorStart(i);
    }

  }//Zusatz Notaus

  //--> Serial.println("Wieder hoch!");


  while (((Position[1] < PosOben) || (Position[2] < PosOben) || (Position[3] < PosOben) || (Position[4] < PosOben)) && (NotAusPressed == false) && (G_MaxFlag == 0)) {

    if (FirstStart != 1) {
      G_MaxHandling();
    }


    for (int i = 1; i <= 4; i++) {
      GetPosition(i);
      delay(10);
      if (Position[i] >= PosOben) {
        MotorHardStop(i);
      }

      if (((Position[i] >= kippPosition) && (KippStatus[i] == 1)) || ((millis() - MagnetOnTime) > MagnetMaxTime)) {
        KippStatus[i] = 0;
        digitalWrite(MagnetPin[i], LOW);
      }
    }
    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);

    if ((NotAusStatus == 1) || (G_MaxFlag == 1)) {
      NotAusHandling();
      for (int k = 1; k <= 4; k++) {
        digitalWrite(MagnetPin[k], LOW);
        KippStatus[k] = 0;
      }
    }
    //--------NOTAUSROUTINE START

  }


  // -------------------------------------------------------------------------------------------------------------------------------
  // Motoren jetzt in der jeweiligen StartPosition
  // -------------------------------------------------------------------------------------------------------------------------------


  Serial2.println("#1o4000"); //Maximalfrequenz Seite50
  delay(10);
  Serial2.println("#2o4000"); //Maximalfrequenz Seite50
  delay(10);
  Serial2.println("#3o4000"); //Maximalfrequenz Seite50
  delay(10);
  Serial2.println("#4o4000"); //Maximalfrequenz Seite50
  delay(10);

  if ((NotAusPressed == false) && (G_MaxFlag == 0)) { //Zusatz Notaus


    for (int i = 1; i <= 4; i++) {
      SetMotorRichtung(i, 0);  //runter
      MotorStart(i);
    }
  } //Zusatz Notaus

  while (((M_Done[1] == false) || (M_Done[2] == false) || (M_Done[3] == false) || (M_Done[4] == false)) && (NotAusPressed == false) && (G_MaxFlag == 0)) {

    if (FirstStart != 1) {
      G_MaxHandling();
    }

    for (int i = 1; i <= 4; i++) {
      GetPosition(i);
      /*if (abs(Position[i]-StartPosition[i]) < Schwellwert){
        MotorHardStop(i);
      }
      else if ((Position[i]<StartPosition[i])&&(Richtung[i] == 0)){
        SetMotorRichtung(i, 1);
      }

      else if ((Position[i]>StartPosition[i])&&(Richtung[i] == 1)){
        SetMotorRichtung(i, 0);
      }
      */
      if (Position[i] < StartPosition[i]) {
        MotorHardStop(i);
      }
      delay(10);

    }

    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);

    if (NotAusStatus == 1) {
      NotAusHandling();
    }
    //--------NOTAUSROUTINE START
  }

  delay(10);
  Serial2.println("#*o10000"); //Maximalfrequenz Seite50
  delay(1000);                 //Ausschwingzeit vor Nullreset Wägezellen

  for (int i = 1; i <= 4; i++) {
    weightRead(i);
    delay(10);
    if (Gewicht[i] < 10) zero(i);
    //delay(10);
    while (Serial3.available() > 0) {
      Serial3.read();
    }
  }


  if ((NotAusPressed == false) && (G_MaxFlag == 0)) { //Zusatz Notaus

    StartWert = 0;
    StatusNeu = 13;
    StatusAlt = 13;


    //SOUND//---------------------------------------------------------------
    //LEDSerial.print("play 1;"); //at first make all sound channel
    delay(50);
    //LICHT//-------------------------------------------------------------------
    LEDSerial.print("kerze;");
    Waagezeit_inaktiv = millis();

    //NotAusPressed = false;

  }  //Zusatz Notaus

  //SOUND//---------------------------------------------------------------
  //LEDSerial.print("sd A90 B90 C90 D90 E90 F90;");
  //volumnTrack(31,-70);  // A
  //volumnTrack(32,-70);  // B
  //volumnTrack(33,-70);  // C
  //volumnTrack(34,-70);  // D
  //volumnTrack(35,-70);  // E
  //SOUND//---------------------------------------------------------------
  //Serial1.write((byte)0x30);
  stop(36);
  //play(30); // only when the finish sound start to play, stop this sound.
  //volumnTrack(30,-5);
  sound(5); //When Rätsel gelöst, dann stop alle gelesene Text. (It didn't works. 2016-3-30).
  delay(50);


  Serial.println("Calibration finished");

}

