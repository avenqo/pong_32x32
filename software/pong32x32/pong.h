#ifndef PONG_H
#define PONG_H
#include "log.h"
#include "Global.h"
#include "MAX7219Display.h"
#include "Joystick.h"
#include "DFPlayer.h"
// #define PONG_DEBUG


class Pong {

public:
  // Requires that Serial is alread
  Pong(MAX7219Display *m, DFPlayer* p);
  //~Pong();

  /**
  Draw initial sprites.
  */
  void clearDisplay();
  void loop(Joystick *j1, Joystick *j2);
  void drawBar(uint8_t y, bool isLeft);
  void drawBall(uint8_t x, uint8_t y);
  void startGame();  // start button pressed

private:
  Log* logg;
  // height of bar
  const uint8_t HEIGHT_BAR = 6;
  DFPlayer* player;

  byte roundCounter;
  MAX7219Display *matrixDisplay;
 
  // y coord of bars (left, right)
  uint8_t posBarLeftY, posBarRightY;
  // position of ball
  uint8_t ballX, ballY;

  int ballSpeedX, ballSpeedY;
  // counter of goals
  uint8_t goalsLeft, goalsRight;
  //bool ballDirection;

// if true, loop should calculate ball position AND show new Result and start a new round.
  bool goalDetected;

  // the ball shall be slower than the paddle
  uint8_t ignoreBall;

  // Draw the graphics
  uint8_t increment(uint8_t i);
  uint8_t decrement(uint8_t i);
  void calcBallPosition();

  bool checkBall4Collision(uint8_t border_x, uint8_t barY, bool isLeft);
  //void goal4Left();
  //void goal4Right();
  void onGoal();

  void startRound();  // After goal
};
#endif
