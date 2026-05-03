#include "DFPlayer.h"

HardwareSerial mySerial(2);  // UART2

DFPlayer::DFPlayer() {
  player = new DFRobotDFPlayerMini();
}

DFPlayer::~DFPlayer() {
}

void DFPlayer::setVolume(uint8_t v) {
  player->volume(v);
}

void DFPlayer::init() {
  // Init serial port for DFPlayer Mini
  mySerial.begin(9600, SERIAL_8N1, PIN_MP3_TX, PIN_MP3_RX);

  // Start communication with DFPlayer Mini
  if (player->begin(mySerial)) {
    Serial.println("DFPlayer initialized");
    player->setTimeOut(500);
    // Set volume to maximum (0 to 30).
    player->volume(10);
    player->outputDevice(DFPLAYER_DEVICE_SD);

    Serial.print("DFPlayer::init() -> readFileCounts(): ");
    Serial.println(player->readFileCounts());

    // player->play(1);
    player->play(37);
    delay(3000);

  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
}

void DFPlayer::startGame() {
  player->play(25);
}

void DFPlayer::startRound() {
  player->play(38);
}

void DFPlayer::onGoal() {
  player->play(7);
}

void DFPlayer::onHitLeftRacket() {
  player->play(15);
}
void DFPlayer::onHitRightRacket() {
  player->play(17);
}
void DFPlayer::onBorderTouched() {
  player->play(27);
}

void DFPlayer::play(int num) {
  #ifdef DFPLAYER_LOG
    Serial.print("DFPlayer::play(");
    Serial.print(num);
    Serial.print(")");
  #endif
  player->play(num);
}