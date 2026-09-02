#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

// Main loop period in ms; the lower the value, the faster the game.
// This is a *delay*, so smaller number == faster ball/paddles.
#define LOOP_TIME_INITIAL 50
// Hard limits for the loop period. The lower bound must stay ABOVE the time
// one full matrix refresh takes: Matrix32x32::drawMemory() pushes 1024 WS2812
// LEDs with interrupts disabled for ~31 ms. If the loop is allowed to run
// faster than that, show() calls pile up back-to-back, interrupts starve and
// the board can brown out / freeze mid-frame. 33 ms ~= 30 fps is the floor.
#define MIN_LOOP_MS 33
#define MAX_LOOP_MS 120
// Never repaint the LED matrix more often than this (ms), whatever the loop does.
#define MIN_DRAW_MS 30
// How many ms one encoder detent changes the loop period in the SPEED menu.
#define SPEED_STEP 4
// of matrix
#define MATRIX_BRIGHTNESS 16
// of matrix
#define VOLUME_VAL 10
// Goals to win a match. Menu offers 3 / 5 / 10.
#define WIN_SCORE_DEFAULT 5
// How strongly the ball speeds up during a rally. 0 = off, 1..3 = gentle..strong.
#define ACCEL_DEFAULT 2

class GraphicsMem {
public:
  static uint8_t screen[32][32][3];
};


class MenuSetup {

public:
  // Loop period in ms (delay). Kept within [MIN_LOOP_MS, MAX_LOOP_MS].
  unsigned int speedValue = LOOP_TIME_INITIAL;
  uint8_t volumeValue = VOLUME_VAL;
  uint8_t brightnessValue = MATRIX_BRIGHTNESS;
  uint8_t winScore = WIN_SCORE_DEFAULT;
  uint8_t accelValue = ACCEL_DEFAULT;
  byte colorValue = 1;
  const char* getStateString(){
    
  }
};

// --- System State ---
enum StateItem { START,
                 DEMO,
                 GAME };

extern const char* stateStrings[];
extern StateItem systemState;

#endif