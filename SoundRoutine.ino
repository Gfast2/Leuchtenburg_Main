/* methods Sound Routine for sound control
 * sound(int) will receive numbers from main programm to control sounds.
 * The numbers sound(int) accept is: 1-Group1, 2-Group2+Group6, 3-Group7,4-group4, 5-stop all playing songs
 *
 *
 * written by: Su Gao
 * Last edite: 28 Apr. 2015
 */

int dauerMap[21] = {
  3, 5, 6, 
  10, 2, 
  2, 4, 4, 
  9, 3, 
  5, 7, 5/*einigt*/, 
  6, 6, 6, 8/*einigt*/, 
  12, 2, 4 // *enigt means there are two sound in the same position from the RAW sound data. The this time long means the longest of the two.
}; //Unit:Sec. track the length of each sentence.

enum soundName {
  s1A = 1, s1B, s1C,
  s2A, s2B,
  s3A, s3B, s3C,
  s4A, s4B,
  s5A, s5B, s5C,
  s6A, s6B, s6C, s6D,
  s7A, s7B, s7C
}; //Song name for each track.

boolean finishTalk = true;    // flagged if one sentence finished to be talked.
unsigned long talkTimer = 0;  // record the start time point of sentences.
int said = 1;                 // last track number be said. equal to '1' only for the first time start system, to get a correct calculation in finishedTalk
int songBuffer[2] = {NULL, NULL};     // FIFO, buffer still not be played sentences.

// return & loop all sentences which is speaked out in the same situation.
// INPUT: which situation have meet.
// OUTPUT: song number to be speak out.
int shuffel(int situation) { //situation should from 1-7
  static int group[7] = {0, 1, 1, 1, 1, 1, 1}; //7 group for 7 situations (sound group).
  const int limit[7] = {3, 2, 3, 2, 3, 4, 3}; //different group have different possibility (number of songs)
  int returnVal = 0;
  if (situation == 1) {
    //group[0] = group[0] > limit[0] ? group[0]++ : 1; //TODO: this line have logical problem.
    group[0]++;
    if (group[0] > limit[0])
      group[0] = 1;
    pl("group[0] now: ");  pt(group[0]);
    return group[0];
  }
  else {
    for (int i = 0; i < (situation - 1); i++) {
      returnVal += limit[i];
    }
    //Serial.print(" returnVal1: ");
    //Serial.println(returnVal);
    returnVal += group[situation - 1];
    //Serial.print(" returnVal2: ");
    //Serial.println(returnVal);
    //the used group song number+1
    //group[situation-1] = group[situation-1] > limit[situation-1] ? group[situation-1]++ : 1;
    group[situation - 1]++;
    if (group[situation - 1] > limit[situation - 1]) {
      group[situation - 1] = 1;
    }
    //    Serial.print("group[situation-1] now: ");
    //    Serial.println(group[situation-1]);
    Serial.print("Now play song: ");
    Serial.println(returnVal);
    return returnVal;
  }
}

//When sound routine jump more than one time in mode number '2' false: play sound in group 2, true: play sound in group 6. After each calibration, this flag is set to false.
boolean trigger26 = false;

//speak out sentense from Mr. Bottinger
//INPUT: The sentense group number (1-groupONE,2-groupTWO+groupSIX, 3-groupSEVEN, 4-groupFOUR) or the spesific number in group3, 4 and 5. That is from 006 to 013. Or 5 to group "nothing" to stop all sound playing.
void say(int track) {
  if (track == 4)
    said = shuffel(4); //loop two sounds in group4
  else if (track == 3) //Sucess sound. In group7
    said = shuffel(track + 4);
  else if (track == 2) {
    if (trigger26 == false)  said = shuffel(track); //false-still talk about sounds in group2
    else                     said = shuffel(track + 4);
  }
  else if (track == 1)
    said = shuffel(track);
  else
    said = track; //directly read the specified sound number (that's the sounds in group 3 to 6)
  play(said);
  finishTalk = false;
  talkTimer = millis();
}

// return if the sentence finished to be read
boolean finishedTalk() {
  if (said == 0) {
    return true;
  } else {
    if (millis() - talkTimer > 1000 * dauerMap[said - 1])
      return true;
    else
      return false;
  }
}

// figure out if the sound state a new one (meaningful one), or a repeated one.
boolean freshSound(int soundState) {
  static boolean freshed = false;
  static int soundState_old = 1; //static, old status from last loop. After each calibration or the main programm start at the first time. group1 sound should be played.
  freshed = soundState == soundState_old ? false : true;
  if (freshed == true) {
    soundState_old = soundState; // update the old one.
  }
  return freshed;
}

// If the new coming sound is importenter then the buffered ones. Set the new coming sound as the next one.
// Return:
// true  :  Priority is handled
// false :  Priority is not handled
boolean handleSoundPriority(int newSound) { // from s4A to s5c is from 009 to 013
  pl("in handleSoundPriority(int)");
  if (newSound == s4A || newSound == s4B || newSound == s5A || newSound == s5B || newSound == s5C) { //these are the importent ones.
    pl("priority handled");
    songBuffer[1] = songBuffer[0]; //push back the old "next song"
    songBuffer[0] = newSound;
    return true;
  }
  else if (said == 0){ //only when the programm first time started, said equal to 1.
    songBuffer[0] = newSound;
    return true;
  }
  else {
    songBuffer[1] = newSound;
    pt("stage 1: songBuffer[0] and songBuffer[1]: ");
    pt(songBuffer[0]);  pt(" ");  pl(songBuffer[1]);
    /*
    if(songBuffer[0] == NULL)  songBuffer[0] = newSound;//Sound Buffer handling.
    else                       songBuffer[1] = newSound;
    pt("stage 2: songBuffer[0] and songBuffer[1]: ");
    pt(songBuffer[0]);  pt(" ");  pl(songBuffer[1]);
    */
    return false;
  }
}

int soundState34_old = s3A; //soundState of the last time.
// In sound group3 and group4. If weight from heavey to less heavey. the sentence shouldn't be read. For example. sound can be read from 6 to 8. But shouldn't from 8 to 6.
// Meanwhile if weight go from very heavy (group4) to less heavy (group3). Sounds in group3 shouldn't be read.
// return true when problem is handled. (When shound should not be read, because the weight is go from heavy to less heavy)
boolean handleGroup34(int soundState) {
  pt("in handleGroup34(int) and return: ");
  if (soundState >= s3A && soundState <= s4B) { //if new coming in sound is in group3 or group4.
    if (soundState < soundState34_old) {
      //soundState34_old = soundState;
      pl("true");
      pt("songState_old = "); pl(soundState34_old);
      pt("songState     = "); pl(soundState    );
      return true; //problem in group3 and group4 is handled.
    } else { //
      soundState34_old = soundState; //soundState is a value in group3 or group4. and is not handled.
      pl("false");
      pt("songState_old = "); pl(soundState34_old);
      pt("songState     = "); pl(soundState    );
      return false;
    }
  }
  soundState34_old = s3A; //When comes to here, soundState is a value nothing to do with group3 and group4. update old soundState to the first sound on group3 (sound with smallest)
  pl("false. (number out of gourp3 and group4.)");
  pt("songState_old = "); pl(soundState34_old);
  pt("songState     = "); pl(soundState    );
  return false;
}

int soundState5_old = s5A; //soundState of the last time.
// In sound group5. If weight from heavey to less heavey. the sentence shouldn't be read. For example. sound can be read from 11 to 13. But shouldn't from 13 to 11.
// Meanwhile if weight go from very heavy (group4) to less heavy (group3). Sounds in group3 shouldn't be read.
// return true when problem is handled. (When shound should not be read, because the weight is go from heavy to less heavy)
boolean handleGroup5(int soundState) {
  pt("in handleGroup5(int) and return: ");
  if (soundState >= s5A && soundState <= s5C) { //if new coming in sound is in group3 or group4.
    if (soundState < soundState5_old) {
      //soundState_old = soundState;
      pl("true");
      pt("songState5_old = "); pl(soundState5_old);
      pt("songState     = "); pl(soundState    );
      return true; //problem in group3 and group4 is handled.
    } else { //
      soundState5_old = soundState; //soundState is a value in group3 or group4. and is not handled.
      pl("false");
      pt("songState_old = "); pl(soundState5_old);
      pt("songState     = "); pl(soundState    );
      return false;
    }
  }
  soundState5_old = s5A; //When comes to here, soundState is a value nothing to do with group3 and group4. update old soundState to the first sound on group3 (sound with smallest)
  pl("false. (number out of gourp5.)");
  pt("songState_old = "); pl(soundState5_old);
  pt("songState     = "); pl(soundState    );
  return false;
}

//stop all playing songs, sofort.
void stopPlaying() {
  stopAll();
  //TODO: reset variable for speak finish check mechanism.
}

const long INTERVAL4 = 20000; //20 seconds.
const long INTERVAL2 = 20000; //20 seconds.
const long INTERVAL1 = 20000; //20 seconds.
unsigned long trigger4Timer = 0; //if mode stay in Ueberladung mode more than 20 Sec. read one time sound 009
unsigned long trigger2Timer = 0; //if mode stay in group2 and group4 mode more than 20 Sec. read one time sound in group2 and group4
unsigned long trigger1Timer = 0; //if mode stay in group1 mode more than 20 Sec. read one time sound in group1
boolean trigger4 = false;   //track if now it's in mode 4
boolean trigger1 = false;   //track if now it's in mode 1
int times26 = 0; //how many times after each calibration it jumped in to situations echo '2' to sound routine.

// Handling new come in new sound
void handleNewSound(int newSound) {
  //TODO:
  // (finish) decide1: if all sounds have been played. If yes, play the newSound sofort
  // (finish) decide2: if there is still sound playing. decide if this new sound is a importent sound, which should replace the sound, that is to be played next (in buffer).
  // (finish) decide3: In falscher Zutat mode, all sounds in this group have their OWN SOUND STATE NUMBERS. Meanwhile these number stands for different weight on this false Zutat plate. 1-less weight, 2-middle, 3-heavy
  // (finish) decide4: In Ueberladung mode, at the first time in this mode, speak the first sound of this group, the second time speak the second one. than the first one again.
  // (finish) decide5: If it is always in Ueberladung mode, the FIRST sound of this group should be repeat every 20 Sec.
  // (finish) decide6: It should have a situation, in which nothing to do. (Stop all playing sound, sofort)
  // (finish) decide7: Sound group 2 and 6 are very like each other. After one calibration. It should be read from the first of group 2 to the last of the group 6. And If going on. Sound in group 6 will be looped.
  // (finish) decide8: Sound group 3 all sounds in this group have their OWN SOUND STATE NUMBERS.Meanwhile these number stands for different weight on this false Zutat plate. 1-less weight, 2-middle, 3-heavy
  // (finish)          And if the weight is lighter again, (heavey->middle->less). In this direction, nothing should be said.
  // (finish) decide9: State jump from sound group 4 to sound group 3 shouldn't trigger sounds in group 3. Because of the logics.
  // (finish) decide10:If it is always in the mode of Sound group 2 and 6. Every 20 Seconds, a new sound of these group should be spoken. That means. When we getting '2' more then 20 sec. We read out next sound in these group.
  // (finish) decide11:Sound in group5 should not read from 13 to 11. But can be read from ascending direction.
  // (finish) decide12:If it is always in group1 sound mode. Every 20 Second (or after a random time) should read out one of sounds in group1.
  // (finish) decide13:Check out the sound buffer system. And let it buffer very correctely.
  pt("in handleNewSound(int). \nnewSound now: ");
  pl(newSound);
  if (newSound == 5) { //if stop all playing stopped.
    pl("newSound=5, stop play all songs");
    stopPlaying();
    finishTalk = true;
    songBuffer[0] = NULL;
    songBuffer[1] = NULL; //TODO: handle the situation when buffer is NULL.
  } else {
    finishTalk = finishedTalk(); //track if the old sound have been finished.
    pl("newSound is not number 5. Check if finishTalk.");
    boolean handled = handleSoundPriority(newSound); //check if the newSound is a importent sound to be speak out.
    if (finishTalk == true) {
      pl("finishTalk");
      //boolean handled = handleSoundPriority(newSound); //check if the newSound is a importent sound to be speak out.
      if (songBuffer[0] != NULL) {
        say(songBuffer[0]);
        songBuffer[0] = NULL;
        if (songBuffer[1] != NULL) {
          songBuffer[0] = songBuffer[1];
          songBuffer[1] = 0;
        }
      }
      /*
      if (handled == false) {
        songBuffer[0] = songBuffer[1];
        songBuffer[1] = newSound;
      }
      */
    }
  }

  /*
  // Still have song to be played + the last Song is finished to be played.
    if(songBuffer[0] != NULL && finishTalk==true){
      songBuffer[0] = songBuffer[1];
      songBuffer[1] = NULL;
      say(songBuffer[0]);
    }
    */
  if (newSound == 9 || newSound == 10 || newSound == 4) {//directly call a specified sound data or '4' for this group.
    trigger4 = true;
    trigger4Timer = millis();
  }
  else
    trigger4 = false;

  if(newSound == 1){
    trigger1 = true;
    trigger1Timer = millis();
  }
  else
    trigger1 = false;

  if (newSound == 2) {
    times26++;
    trigger2Timer = millis();
  }
}

// If it stay in group4 more than 20 seconds. It should read first sound of this group onces again.
void handleGroup4() {
  if (trigger4 == true) {
    if (millis() - trigger4Timer > INTERVAL4) {
      say(s4A); //first song of group4, number 9, It ignore the sound buffer. But it's more than 20 Sec. It should not have any not played song in the buffer.
      trigger4Timer = millis(); //restart the timer again.
    }
  }
}

// It has the same functionality as handleGroup1(). Tha means, if in Group1 sound mode more than 20 sec. The one of three sound in group1 will read out one time.
void handleGroup1(){
  if (trigger1 == true) {
    if (millis() - trigger1Timer > INTERVAL1) {
      say(1); //first song of group4, number 9, It ignore the sound buffer. But it's more than 20 Sec. It should not have any not played song in the buffer.
      trigger1Timer = millis(); //restart the timer again.
    }
  }
}

// If there is no new songs comes in. This routine will handle if play next songs and some other stuff
void handleSong() {
  finishTalk = finishedTalk();
  if (finishTalk == true) {
    /*
    if(songBuffer[0] != NULL){
      say(songBuffer[0]);
      songBuffer[0] = NULL;

    //      if(songBuffer[1] != NULL){
    //        songBuffer[0] = songBuffer[1];
    //        songBuffer[1] = NULL;
    //      }
    }
    if(songBuffer[1] != NULL){
      songBuffer[0] = songBuffer[1];
      songBuffer[1] = NULL;
      //say(songBuffer[0]);
    }
    */
    if (songBuffer[0] != NULL) {
      say(songBuffer[0]);
      songBuffer[0] = NULL;
      if (songBuffer[1] != NULL) {
        songBuffer[0] = songBuffer[1];
        songBuffer[1] = NULL;
      }
    }
    else if (songBuffer[1] != NULL) {
      songBuffer[0] = songBuffer[1];
      songBuffer[1] = NULL;
      say(songBuffer[0]);
      songBuffer[0] = NULL;
    }
  }
}

// After each calibration, If the situation satisfied, it will start to read the first sound of group2 through to the last of group6. And loop in group6 all the time, when it is always in the same situation.
void handleGroup26(int soundState) {
  if (times26 > 2)
    trigger26 = true;
  if (millis() - trigger2Timer > INTERVAL2 && soundState == 2) {
    //TODO:
    handleNewSound(2); //read next sounds in group2 and group4.
  }
}

// Sound triggers that should be reset after each new calibrations.
void soundFlagesReset() {
  trigger26 = false; // restart saying sounds from group2
  finishTalk = true; // all sounds have been speaked out
  times26 = 0;       // reset counter back to 0
  soundState34_old = s3A; // in handleGroup34(int). update old soundState to the first sound on group3 (sound with smallest)
  soundState5_old  = s5A;
}

// the main sound routine.
// INPUT: Sound Status number, which descript the Sound status for now.
void sound(int soundState) {
  //Serial.println(F("In sound(int) now"));
  pl("In sound(int) now");
  if (freshSound(soundState) == true) //only when new sound routine comes in, we will go on handling stuff.
  {
    pl("freshSound(int) is true");
    boolean handled34 = handleGroup34(soundState);
    boolean handled5  = handleGroup5(soundState);
    if (!handled34 && !handled5){
      pl("new song handled.");
      handleNewSound(soundState); //When there is new songs comes in. Process new songs and play the next song if it is the correct time point.
    }
  } else {
    pl("old song processed.");
    handleSong(); //When there is no new songs comes in. process sounds buffer.
  }
  handleGroup4();
  handleGroup26(soundState);
  handleGroup1();

  pt("songBuffer[0] & songBuffer[1]: ");
  pt(songBuffer[0]);  pt(" ");  pl(songBuffer[1]);
}

// method to control five different ambient sounds.
void soundAmb(int amb){
  switch(amb){
    case 1:
      volumnTrack(31,-70);  // A
      volumnTrack(32,-70);  // B
      volumnTrack(33,-70);  // C
      volumnTrack(34,-70);  // D
      volumnTrack(35,-70);  // E
      break;
    case 2:
      volumnTrack(34,-5);  // D
      volumnTrack(35,-70); // E
      break;
    case 3:
      volumnTrack(34,-70); // D
      volumnTrack(35,-5);  // E
      break;
    case 4:
      volumnTrack(34,-70);  // D
      volumnTrack(35,-70);  // E      
      break;
    case 5:
      volumnTrack(31,-5);  // A
      volumnTrack(32,-5);  // B
      volumnTrack(33,-5);  // C
      break;
    case 6:
      volumnTrack(31,-5);  // A
      volumnTrack(32,-5);  // B
      volumnTrack(33,-70); // C    
      break;
    case 7:
      volumnTrack(31,-5);  // A
      volumnTrack(32,-70);  // B
      volumnTrack(33,-70);  // C    
      break;
    case 8:
      volumnTrack(31,-70);  // A
      volumnTrack(32,-70);  // B
      volumnTrack(33,-70);  // C
      break;
    default:
      Serial.println(F("soundAmb(int) has a unguilty value"));
  }
  
}


//simplified code notize.Strings in "F(String)" will be not loaded directely in memonry, only when they is needed. In order to save memonry.
void pl(String note){
  //Serial.println(F(note));
  Serial.println(note);
}

void pl(int note){
  //Serial.println(F(note));
  Serial.println(note);
}

//the same as pl(Sring). But without a '\n' at the end of it.
void pt(String note){
  Serial.print(note);
  //printf_P(PSTR(note));
}

void pt(int note){
  Serial.print(note);
  //printf_P(PSTR(note));
}
