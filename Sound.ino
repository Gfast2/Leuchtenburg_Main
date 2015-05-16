/* WAVE Trigger Board control methods.
 * Couple simple methods to let Arduino communicate with WAVE Trigger Board.
 * Product description     : https://www.sparkfun.com/products/12897
 * Serial control reference: http://robertsonics.com/wav-trigger-online-user-guide/
 *
 *
 * Written by: Su Gao
 * Last Edite: 23.April 2015
 */

enum state {ON = 0x05, OFF};

//Start-of-Message(SOM) header
void som() {
  Serial1.write(0xf0);
  Serial1.write(0xaa);
}

//End-of-Message(EOM)
void eom() {
  Serial1.write(0x55);
}

//Play track
void play(int track) {
  som();
  Serial1.write(0x08);               //indicate the total number of bytes in the message
  Serial1.write(0x03);               // message code, range of 0x00 to 0x7f, here is CONTROL_TRACK
  Serial1.write(0x01);               //data * n.(LSB first and the MSB second), Play track polyphonically
  byte msb = (track & 0xFF00) >> 8;
  byte lsb =  track & 0x00FF;
  Serial1.write(lsb);                //LSB, song number is up to 999 limited (hardware limitation).
  Serial1.write(msb);                //MSB, http://forum.arduino.cc/index.php?topic=45769.0
  eom();
}

//Play track
void playSolo(int track) {
  som();
  Serial1.write(0x08);
  Serial1.write(0x03);
  Serial1.write((byte)0x00);
  byte msb = (track & 0xFF00) >> 8;
  byte lsb =  track & 0x00FF;
  Serial1.write(lsb);
  Serial1.write(msb);
  eom();
}

//Stop track
void stop(int track) {
  som();
  Serial1.write(0x08);
  Serial1.write(0x03);
  Serial1.write(0x04);
  byte msb = (track & 0xFF00) >> 8;
  byte lsb =  track & 0x00FF;
  Serial1.write(lsb);
  Serial1.write(msb);
  eom();
}

//stop all
void stopAll() {
  som();
  Serial1.write(0x05);
  Serial1.write(0x04);
  eom();
}

//Track loop play. state=1 means loop the sound track.
void trackLoop(int track, int state) {
  som();
  Serial1.write(0x08);
  Serial1.write(0x03);
  Serial1.write(state == ON ? ON : OFF);
  byte msb = (track & 0xFF00) >> 8;
  byte lsb =  track & 0x00FF;
  Serial1.write(lsb);
  Serial1.write(msb);
  eom();
}

//change master volume
void volume(int volumn) {
  som();
  Serial1.write(0x07);
  Serial1.write(0x05);
  byte msb = (volumn & 0xFF00) >> 8;
  byte lsb =  volumn & 0x00FF;
  Serial1.write(lsb);
  Serial1.write(msb);
  eom();
}

//change track volume
void volumnTrack(int track, int volumn) {
  som();
  Serial1.write(0x09);
  Serial1.write(0x08);
  byte msb_track = (track & 0xFF00) >> 8;
  byte lsb_track =  track & 0x00FF;
  Serial1.write(lsb_track);
  Serial1.write(msb_track);
  byte msb = (volumn & 0xFF00) >> 8;
  byte lsb =  volumn & 0x00FF;
  Serial1.write(lsb);
  Serial1.write(msb);
  eom();
}

//En-/Disable on board AMP
void ampEnable(boolean on) {
  som();
  Serial1.write(0x06);                 // indicate the total number of bytes in the message
  Serial1.write(0x09);                 // message code, range of 0x00 to 0x7f, here is CONTROL_TRACK
  Serial1.write(on == true ? 1 : 0);   // data * n.(LSB first and the MSB second), Play track polyphonically
  eom();
}

//Track fade
void trackFade(int track, int volumn, long milis) {
  som();
  Serial1.write(0x0C);
  Serial1.write(0x0a);
  byte msb_track = (track & 0xFF00) >> 8;
  byte lsb_track =  track & 0x00FF;
  Serial1.write(lsb_track);
  Serial1.write(msb_track);
  byte msb = (volumn & 0xFF00) >> 8;
  byte lsb =  volumn & 0x00FF;
  Serial1.write(lsb);
  Serial1.write(msb);
  byte msb_milis = (milis & 0xFF00) >> 8;
  byte lsb_milis =  milis & 0x00FF;
  Serial1.write(lsb_milis);
  Serial1.write(msb_milis);
  Serial1.write((byte)0x00); // don't stop
  eom();
}



