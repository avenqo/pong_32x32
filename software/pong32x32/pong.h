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
  Pong(MAX7219Display *m, DFPlayer *p);
  //~Pong();

  /**
  Draw initial sprites.
  */
  void clearDisplay();
  void loop(Joystick *j1, Joystick *j2);
  void drawBar(uint8_t y, bool isLeft);
  void drawBall(uint8_t x, uint8_t y);
  void startGame();  // called whenever DEMO / GAME is (re)entered
  void calcDemoInput();
  void enableDemoMode() { demoMode = true; }
  void disableDemoMode() { demoMode = false; }
  // Goals needed to win a match (menu: 3 / 5 / 10, default 5).
  void setWinScore(uint8_t n) { winScore = n; }
  // Rally speed-up strength (menu: 0 = off, 1..3 = gentle..strong).
  void setAccel(uint8_t n) { accelLevel = n; }

private:
  Log *logg;
  bool demoMode = false;

  // --- Non-blocking round flow --------------------------------------------
  // Replaces the old delay()-driven startGame()/startRound()/onGoal() so the
  // joysticks and the encoder stay responsive the whole time.
  enum Phase { PH_INTRO,
               PH_COUNTDOWN,
               PH_PLAYING,
               PH_GOAL,
               PH_MATCH_OVER };
  Phase phase = PH_INTRO;
  unsigned long phaseStart = 0;
  int lastCountShown = -1;

  static constexpr unsigned long INTRO_MS = 1500;
  static constexpr unsigned long GOAL_MS = 2200;
  static constexpr unsigned long MATCH_OVER_MS = 6000;
  static constexpr int COUNTDOWN_FROM = 3;

  uint8_t winScore = WIN_SCORE_DEFAULT;   // first to this many goals wins the match
  uint8_t accelLevel = ACCEL_DEFAULT;     // 0 = ball never speeds up, 1..3 = ramp

  // "Fake Input"
  uint8_t demo_up_left = 0, demo_down_left = 0;
  uint8_t demo_up_right = 0, demo_down_right = 0;

  // Demo AI: aim a few pixels off-centre and re-roll that error every so often
  // so rallies do not lock into an endless flat exchange.
  uint16_t demoTick = 0;
  int demoOffsetLeft = 0, demoOffsetRight = 0;

  // height of bar
  const uint8_t HEIGHT_BAR = 6;
  DFPlayer *player;

  byte roundCounter;
  MAX7219Display *matrixDisplay;

  // y coord of bars (left, right)
  uint8_t posBarLeftY, posBarRightY;
  // position of ball (top-left pixel of the 2x2 ball)
  uint8_t ballX, ballY;

  int ballSpeedX, ballSpeedY;
  // paddle hits in the current rally -> the ball speeds up (QW3)
  uint8_t rallyHits;
  // counter of goals
  uint8_t goalsLeft, goalsRight;
  //bool ballDirection;

  // set by the ball step when a goal line is reached
  bool goalDetected;

  // the ball shall be slower than the paddle: only step it every Nth loop
  uint8_t ignoreBall;

  // --- Ball trail (QW4) ---
  static constexpr uint8_t TRAIL_LEN = 5;
  uint8_t trailX[TRAIL_LEN];
  uint8_t trailY[TRAIL_LEN];
  uint8_t trailHead;
  uint8_t trailCount;

  // paddle movement
  int paddleStep(uint8_t upLevel, uint8_t downLevel);
  uint8_t moveBar(uint8_t y, int step);
  void handlePaddles(Joystick *jl, Joystick *jr);

  // ball movement (swept / sub-stepped so a fast ball cannot tunnel)
  void calcBallPosition();
  bool advanceBallUnit(int dx, int dy);  // returns true when the tick must stop
  int ballStepX();
  bool checkBall4Collision(uint8_t border_x, uint8_t barY, bool isLeft);
  bool ballOverlapsBar(uint8_t barTopY);
  void onPaddleHit();

  // round flow
  void enterCountdown();
  void serveBall();
  void onGoal();
  void enterMatchOver();

  // rendering helpers
  void drawNet();
  void pushTrail(uint8_t x, uint8_t y);
  void clearTrail();
  void drawTrail();
  void drawCountdownDigit(int n);
  void drawMatchOver(unsigned long now);
};
#endif
