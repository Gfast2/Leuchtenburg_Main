void Errorhandling(){
  if (FirstTimeError == 0) {
    FirstTimeError = 1;
    //LICHT//-------------------------------------------------------------------
    //LEDSerial.print("lt E0;");
    LEDSerial.print("lt A0 B0 C0 D0;");
    if (ZuSchwer[1] == true)
      LEDSerial.print("lt KA A3000;");
    if (ZuSchwer[2] == true)
      LEDSerial.print("lt KB B3000;");
    if (ZuSchwer[3] == true)
      LEDSerial.print("lt KC C3000;");
    if (ZuSchwer[4] == true)
      LEDSerial.print("lt KD D3000;");
  }
  else if (FirstTimeError == 1) {
    FirstTimeError = 2;
  }
  Serial2.println("#*S0");
  delay(10);
  for (int i = 1; i <= 4; i++) {
    M_Done[i] = true;
  }
}

void NotAusHandling(){
  delay(10);
  NotAusStatus = digitalRead(NotAusPin);
  if (NotAusStatus == 1) {
    if (NotAusPressed == false) {
      NotAusPressed = true;
      MotorHardStop(1);
      MotorHardStop(2);
      MotorHardStop(3);
      MotorHardStop(4);
      //LICHT//-------------------------------------------------------------------
      LEDSerial.print("lt E0;");
    }
  }
  else if (NotAusPressed == true) {
    NotAusPressed = false;
    StartWert = 1;
  }
}

void G_MaxHandling() {
  for (int i = 1; i <= 4; i++) {
    weightRead(i);
    delay(5);
  }
  if ((Gewicht[1] > (G_Max + G_Max_Calib)) || (Gewicht[2] > (G_Max + G_Max_Calib)) || (Gewicht[3] > (G_Max + G_Max_Calib)) || (Gewicht[4] > (G_Max + G_Max_Calib))) {
    MotorHardStop(1);
    MotorHardStop(2);
    MotorHardStop(3);
    MotorHardStop(4);
    G_MaxFlag = 1;
    StartWert = 1;
  } else {
    G_MaxFlag = 0;
  }  
}

void CheckForError(int j)
{
  PositionAlt[j] = Position[j];
  GetPosition(j);
  if (CalibError[j] == false) {
    if (Richtung[j] == 1) {
      if (abs(PositionAlt[j]) - abs(Position[j]) < 0) {
        MotorHardStop(j);
        CalibError[j] = true;
        CalibErrorPause[j] = millis();

        //if (KippStatus[j] == 1){
        //   KippStatus[j] = 0;
        //   digitalWrite(MagnetPin[j],LOW);
        //}
      }
    }
  }
  else if (millis() - CalibErrorPause[j] > 3000) {
    CalibError[j] = false;
    MotorStart(j);
  }
}
