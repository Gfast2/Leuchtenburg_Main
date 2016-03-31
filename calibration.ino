void calibration(){

  LEDSerial.print("kerze;");       //"kerze" bedeutet flackern
  delay(100);  
  soundAmb(1);
  delay(50);

  Waagezeit_inaktiv = millis();  
    
  LEDSerial.print("lt E0;");    //alle Lichter ausschalter
  delay(1000);

  ZustandFaktor = 0;

  SetMotorStartValues();

  //--------NOTAUSROUTINE START
  NotAusStatus = digitalRead(NotAusPin);  
  if ((NotAusStatus == 1)||(NotAusPressed == true))  NotAusHandling(); 
  //--------NOTAUSROUTINE START

  for (int i=1; i<=4;i++){    
    EndSchalter[i]=digitalRead(EndSchalterPin[i]);
    GetPosition(i);

    //Addon NotAus Start   
    if ((EndSchalter[i] == 1) && (NotAusPressed == false) && (G_MaxFlag == 0))  MotorStart(i);
    //Addon NotAus Ende      
  }
    
  while(((EndSchalter[1] == 1)||(EndSchalter[2] == 1)||(EndSchalter[3] == 1)||(EndSchalter[4] == 1))&&(NotAusPressed == false) && (G_MaxFlag == 0)) {    
    if (FirstStart != 1)  G_MaxHandling();      
    for (int i=1;i<=4;i++){
      EndSchalter[i] = digitalRead(EndSchalterPin[i]);
      if (EndSchalter[i] == 0)  MotorHardStop(i);    
    }    
    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);  
    if ((NotAusStatus == 1)||(NotAusPressed == true))  NotAusHandling();
    //--------NOTAUSROUTINE START    
  }


  // -------------------------------------------------------------------------------------------------------------------------------
  // Motoren jetzt ganz oben
  // -------------------------------------------------------------------------------------------------------------------------------
  Serial2.println("#*o7000"); //Maximalfrequenz Seite50
  delay(10);
  Serial2.println("#*D0");    //Drehüberwachung Reset
  delay(10);

  if ((NotAusPressed == false) && (G_MaxFlag == 0)){  //Zusatz Notaus  
    for (int i = 1; i<=4; i++){
      SetMotorRichtung(i, 0);
      KippStatus[i] == 0;
      MotorStart(i);
    }
  }   //Zusatz Notaus

  // -------------------------------------------------------------------------------------------------------------------------------
  // Motoren jetzt ganz unten und ausgeleert
  // -------------------------------------------------------------------------------------------------------------------------------

  while(
  ( (Position[1] > kippPosUnten) || 
    (Position[2] > kippPosUnten) || 
    (Position[3] > kippPosUnten) || 
    (Position[4] > kippPosUnten)
  ) &&
  (NotAusPressed == false) &&
  (G_MaxFlag == 0))
  {    
    if(FirstStart != 1)  G_MaxHandling();        
    for (int i=1; i<=4; i++){
      GetPosition(i);
      delay(10);
      if ((Position[i] <= kippPosition) && (KippStatus[i] == 0)) {
        KippStatus[i] = 1; 
        digitalWrite(MagnetPin[i],HIGH);
      }    
      if (Position[i] <= kippPosUnten)  MotorHardStop(i);
    }
    
    MagnetOnTime = millis();

    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);  
    if ((NotAusStatus == 1)||(G_MaxFlag == 1)){
      NotAusHandling();
      for (int k=1;k<=4;k++){
        digitalWrite(MagnetPin[k],LOW);
        KippStatus[k] = 0;  
      }
    }
    //--------NOTAUSROUTINE START
  }
    
  //----Eingeschoben ZeroSetting
  if ((NotAusPressed == false) && (G_MaxFlag == 0)){  //Zusatz Notaus
    delay(1000);
    for (int i=1; i<=4;i++){
      delay(10);
      zero(i);
      while(Serial3.available() > 0)  Serial3.read();
    }
  }
  //----Ende Einschub ZeroSetting
    
  if ((NotAusPressed == false) && (G_MaxFlag == 0)){  // Zusatz Notaus  
    for (int i=1;i<=4;i++){
      SetMotorRichtung(i, 1);
      MotorStart(i);
    }  
  }  // Zusatz Notaus
    
  while(
  ( (Position[1] < PosOben) || 
    (Position[2] < PosOben) || 
    (Position[3] < PosOben) || 
    (Position[4] < PosOben)) && 
  (NotAusPressed == false)   && 
  (G_MaxFlag == 0)){
    
    if (FirstStart != 1)  G_MaxHandling();  

    for (int i=1; i<=4;i++){
      GetPosition(i);
      delay(10);
      if (Position[i] >= PosOben)  MotorHardStop(i);
      if (((Position[i] >= kippPosition) && (KippStatus[i] == 1)) ||
         ((millis()-MagnetOnTime) > MagnetMaxTime)){
        KippStatus[i] = 0;
        digitalWrite(MagnetPin[i],LOW);
      }
    }
      
    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);
    
    if ((NotAusStatus == 1)||(G_MaxFlag == 1)){
      NotAusHandling();
      for (int k=1;k<=4;k++){
        digitalWrite(MagnetPin[k],LOW);
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

  if ((NotAusPressed == false)&&(G_MaxFlag == 0)){  //Zusatz Notaus  
    for (int i=1;i<=4;i++){
      SetMotorRichtung(i, 0);  //runter
      MotorStart(i);
    }
  } //Zusatz Notaus

  while (
  ( (M_Done[1] == false) || 
    (M_Done[2] == false) || 
    (M_Done[3] == false) || 
    (M_Done[4] == false))  &&
  (NotAusPressed == false) &&
  (G_MaxFlag == 0)){
    
    if (FirstStart != 1)  G_MaxHandling();  
     
    for (int i=1; i<=4;i++){
      GetPosition(i);            
      if (Position[i] < StartPosition[i])  MotorHardStop(i);
      delay(10);
    }
    
    //--------NOTAUSROUTINE START
    NotAusStatus = digitalRead(NotAusPin);    
    if (NotAusStatus == 1)  NotAusHandling();
    //--------NOTAUSROUTINE START  
  }
    
  delay(10);
  Serial2.println("#*o10000"); //Maximalfrequenz Seite50
  delay(1000);                 //Ausschwingzeit vor Nullreset Wägezellen      
 
  for (int i=1; i<=4;i++){
    weightRead(i);
    delay(10);
    if (Gewicht[i] < 10) zero(i);
    while(Serial3.available() > 0)    Serial3.read();
  }
 
  if ((NotAusPressed == false)&&(G_MaxFlag == 0)){  //Zusatz Notaus
    StartWert = 0;
    StatusNeu = 13;
    StatusAlt = 13;

    LEDSerial.print("kerze;");  
    Waagezeit_inaktiv = millis();    
  }  //Zusatz Notaus

  soundAmb(1);
  stop(36); //When Rätsel gelöst, dann stop alle gelesene Text. (It didn't works. 2016-3-30).
  sound(5);
  delay(50);

  Serial.println("Calibration finished");   
}

