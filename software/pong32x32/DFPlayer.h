#include <sys/_stdint.h>
#ifndef DFPLAYER_H
#define DFPLAYER_H

#define DFPLAYER_LOG

// Minimum gap (ms) between two rapid sound effects. Corner rallies can trigger
// paddle + border hits many times per second; the 9600-baud link to the module
// and the module itself cannot keep up, so bursts are collapsed to one.
#define SFX_MIN_GAP_MS 45

#include "HardwareSerial.h"
#include "DFRobotDFPlayerMini.h"

extern HardwareSerial mySerial;  

class DFPlayer {

public:
  // Requires that Serial is alread
  DFPlayer();
  ~DFPlayer();

  void init();
  void setVolume(uint8_t v);
  void play(int num);      // fire-and-forget, never blocks the game loop
  void playSfx(int num);   // like play(), but rate-limited (SFX_MIN_GAP_MS)

  void startGame();
  void startRound();
  void onCountdownTick();  // one beep per "3", "2", "1"
  void onCountdownGo();    // "GO!" when the ball is served
  void onGoal();
  void onMatchWon();       // fanfare when a player reaches the target score
  void onHitLeftRacket();
  void onHitRightRacket();
  void onBorderTouched();

private:
// Use this pins communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 4; // Connects to module's RX 
static const uint8_t PIN_MP3_RX = 13; // Connects to module's TX 

//SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Create the Player object
DFRobotDFPlayerMini *player;

// millis() of the last rate-limited effect (see playSfx / SFX_MIN_GAP_MS)
unsigned long lastSfxMs = 0;

};

#endif
