#include <sys/_stdint.h>
#ifndef DFPLAYER_H
#define DFPLAYER_H

#define DFPLAYER_LOG

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
  void play(int num);

  void startGame();
  void startRound();
  void onGoal();
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

};

#endif
