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
  // Init serial port for DFPlayer Mini (pin order left as wired - sound works).
  mySerial.begin(9600, SERIAL_8N1, PIN_MP3_TX, PIN_MP3_RX);

  // isACK = false: do NOT wait for an acknowledge frame after every command.
  // The RX line from the module is not usable here (readFileCounts() == -1),
  // so an ACK would never arrive and every play() would stall on a timeout.
  // With isACK off, commands are pure fire-and-forget.
  if (player->begin(mySerial, /*isACK=*/false, /*doReset=*/true)) {
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

// TODO: point these at dedicated countdown clips on the SD card once recorded.
// For now we reuse existing blips so the countdown is audible.
void DFPlayer::onCountdownTick() {
  player->play(27);
}

void DFPlayer::onCountdownGo() {
  player->play(38);
}

void DFPlayer::onGoal() {
  play(7);
  // A goal always plays; silence any paddle/border blip that would follow it
  // in the next few ms so the goal sound is not immediately cut off.
  lastSfxMs = millis();
}

// TODO: give the match winner its own fanfare clip; reuses the start jingle now.
void DFPlayer::onMatchWon() {
  player->play(25);
}

void DFPlayer::onHitLeftRacket() {
  playSfx(15);
}
void DFPlayer::onHitRightRacket() {
  playSfx(17);
}
void DFPlayer::onBorderTouched() {
  playSfx(27);
}

// Collapse rapid bursts (corner: paddle hit + wall bounce in the same tick,
// repeating every tick) to at most one command per SFX_MIN_GAP_MS.
void DFPlayer::playSfx(int num) {
  unsigned long now = millis();
  if (now - lastSfxMs < SFX_MIN_GAP_MS)
    return;
  lastSfxMs = now;
  play(num);
}

void DFPlayer::play(int num) {
  #ifdef DFPLAYER_LOG
    Serial.print("DFPlayer::play(");
    Serial.print(num);
    Serial.print(")");
  #endif
  // Never let a full UART TX buffer stall the game loop (e.g. right after a
  // ~31 ms matrix refresh with interrupts disabled). A command is 10 bytes;
  // if the buffer cannot take it, drop this sound rather than block.
  if (mySerial.availableForWrite() < 16)
    return;
  player->play(num);
}