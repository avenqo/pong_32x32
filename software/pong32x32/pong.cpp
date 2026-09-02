#include "pong.h"

// 3x5 pixel glyphs for the countdown digits 3, 2, 1.
// One row per byte, bit2..bit0 = left..right pixel.
static const uint8_t COUNT_GLYPH[3][5] = {
  { 0b111, 0b001, 0b111, 0b001, 0b111 },  // 3
  { 0b111, 0b001, 0b111, 0b100, 0b111 },  // 2
  { 0b010, 0b110, 0b010, 0b010, 0b111 },  // 1
};

// ------- Ctor ------

Pong::Pong(MAX7219Display *m, DFPlayer *p) {
  matrixDisplay = m;
  player = p;

  ballX = 15;
  ballY = 15;

  posBarLeftY = 13;
  posBarRightY = 13;

  ballSpeedX = 1;
  ballSpeedY = 0;

  rallyHits = 0;
  goalsLeft = 0;
  goalsRight = 0;
  goalDetected = false;
  roundCounter = 0;

  phase = PH_INTRO;
  phaseStart = millis();
  lastCountShown = -1;

  clearTrail();

  logg = new Log("Pong");
  ignoreBall = 2;
}

// ------- Loop ------

void Pong::loop(Joystick *j_left, Joystick *j_right) {
  unsigned long now = millis();

  clearDisplay();

  switch (phase) {

    // "PöNG" splash, paddles centred, nothing moves yet.
    case PH_INTRO:
      drawBar(posBarLeftY, true);
      drawBar(posBarRightY, false);
      drawNet();
      matrixDisplay->showMessage(goalsLeft, ":", goalsRight);
      if (now - phaseStart >= INTRO_MS)
        enterCountdown();
      break;

    // 3 - 2 - 1 on the matrix, one beep per number, paddles already movable.
    case PH_COUNTDOWN: {
      int remaining = COUNTDOWN_FROM - (int)((now - phaseStart) / 1000);

      if (remaining != lastCountShown) {
        lastCountShown = remaining;
        if (remaining >= 1)
          player->onCountdownTick();
      }

      handlePaddles(j_left, j_right);

      drawBar(posBarLeftY, true);
      drawBar(posBarRightY, false);
      if (remaining >= 1)
        drawCountdownDigit(remaining);
      drawNet();
      matrixDisplay->showMessage(goalsLeft, ":", goalsRight);

      if (remaining < 1) {
        serveBall();
        phase = PH_PLAYING;
        phaseStart = now;
      }
      break;
    }

    case PH_PLAYING: {
      // Move the paddles FIRST, so a ball step in this same frame is tested
      // against the paddle position the player actually sees, not last frame's.
      handlePaddles(j_left, j_right);

      // Ball is stepped only every 2nd loop so it stays slower than the paddles.
      if (--ignoreBall == 0) {
        ignoreBall = 2;
        calcBallPosition();
        if (!goalDetected)
          pushTrail(ballX, ballY);
      }

      drawTrail();
      drawBar(posBarLeftY, true);
      drawBar(posBarRightY, false);
      drawBall(ballX, ballY);
      drawNet();
      matrixDisplay->showMessage(goalsLeft, ":", goalsRight);

      if (goalDetected) {
        goalDetected = false;
        onGoal();
      }
      break;
    }

    // "TOOOR!" shown for a moment, last frame frozen, then next countdown
    // (or the end-of-match screen if someone just reached the target score).
    case PH_GOAL:
      drawBar(posBarLeftY, true);
      drawBar(posBarRightY, false);
      drawBall(ballX, ballY);
      drawNet();
      matrixDisplay->showMessage("TOOOR!");
      if (now - phaseStart >= GOAL_MS) {
        if (goalsLeft >= winScore || goalsRight >= winScore) {
          enterMatchOver();
        } else {
          matrixDisplay->showMessage(goalsLeft, ":", goalsRight);
          enterCountdown();
        }
      }
      break;

    // Winner celebration, then a fresh match starts by itself (so demo mode
    // keeps looping through complete matches too).
    case PH_MATCH_OVER:
      drawMatchOver(now);
      matrixDisplay->showMessage(goalsLeft, ":", goalsRight);
      if (now - phaseStart >= MATCH_OVER_MS)
        startGame();
      break;
  }
}

// ------- Game / Round flow ------

void Pong::startGame() {
#ifdef PONG_DEBUG
  Serial.println("pong.startGame()");
#endif
  player->startGame();

  goalsLeft = 0;
  goalsRight = 0;
  roundCounter = 0;
  rallyHits = 0;
  goalDetected = false;
  clearTrail();

  matrixDisplay->showMessage("P\xD6NG");
  phase = PH_INTRO;
  phaseStart = millis();
  lastCountShown = -1;
}

void Pong::enterCountdown() {
  phase = PH_COUNTDOWN;
  phaseStart = millis();
  lastCountShown = -1;

  posBarLeftY = 13;
  posBarRightY = 13;
  ballX = 15;
  ballY = 15;
  goalDetected = false;
  clearTrail();
}

void Pong::serveBall() {
#ifdef PONG_DEBUG
  Serial.println("pong.serveBall()");
#endif
  // surprise y position & direction, but never start from the borders
  ballY = random(6, 25);
  ballSpeedY = random(-2, 3);          // -2 .. 2

  ballSpeedX = random(0, 2) * 2 - 1;   // -1 or +1
  ballX = (ballSpeedX > 0) ? 6 : 24;

  rallyHits = 0;
  ignoreBall = 2;
  clearTrail();
  roundCounter++;

  demoTick = 0;
  demoOffsetLeft = random(-3, 4);
  demoOffsetRight = random(-3, 4);

  player->onCountdownGo();
}

void Pong::onGoal() {
  player->onGoal();
  phase = PH_GOAL;
  phaseStart = millis();
}

void Pong::enterMatchOver() {
  player->onMatchWon();
  phase = PH_MATCH_OVER;
  phaseStart = millis();
}

// ------- Paddles ------

void Pong::handlePaddles(Joystick *j_left, Joystick *j_right) {
  if (demoMode) {
    calcDemoInput();
    if (demo_up_left > 0) posBarLeftY = moveBar(posBarLeftY, -1);
    else if (demo_down_left > 0) posBarLeftY = moveBar(posBarLeftY, 1);

    if (demo_up_right > 0) posBarRightY = moveBar(posBarRightY, -1);
    else if (demo_down_right > 0) posBarRightY = moveBar(posBarRightY, 1);
    return;
  }

  // The joystick now reports a 0..3 strength, so pushing further moves the
  // paddle faster (up to 3 px per step) instead of a fixed 1 px.
  posBarLeftY = moveBar(posBarLeftY,
                        paddleStep(j_left->getMoveValue_Up(), j_left->getMoveValue_Down()));
  posBarRightY = moveBar(posBarRightY,
                         paddleStep(j_right->getMoveValue_Up(), j_right->getMoveValue_Down()));
}

int Pong::paddleStep(uint8_t upLevel, uint8_t downLevel) {
  if (upLevel > 0) return -(int)upLevel;    // up   -> smaller y
  if (downLevel > 0) return (int)downLevel; // down -> larger y
  return 0;
}

uint8_t Pong::moveBar(uint8_t y, int step) {
  const int limit = 32 - HEIGHT_BAR;  // top edge may not exceed this
  int ny = (int)y + step;
  if (ny < 0) ny = 0;
  if (ny > limit) ny = limit;
  return (uint8_t)ny;
}

// ------- Ball ------

// Signed x displacement for one ball step. During a rally the magnitude grows
// from 1 up to 4 and resets to slow on the next serve (QW3). How quickly it
// ramps is set by the "Accel" menu: level 0 disables it, level 1..3 adds one
// step every 3 / 2 / 1 paddle hits.
int Pong::ballStepX() {
  int mag = 1;
  if (accelLevel > 0) {
    int hitsPerStep = 4 - accelLevel;  // 3, 2 or 1
    mag = 1 + rallyHits / hitsPerStep;
    if (mag > 4) mag = 4;
  }
  return (ballSpeedX > 0) ? mag : -mag;
}

// One ball "tick": break the (dx,dy) move into single-pixel steps and run the
// collision checks on every pixel. A fast ball can therefore never jump over a
// paddle or through the top/bottom wall.
void Pong::calcBallPosition() {
  int dx = ballStepX();   // magnitude 1..4
  int dy = ballSpeedY;    // -2 .. 2

  int adx = dx < 0 ? -dx : dx;
  int ady = dy < 0 ? -dy : dy;
  int steps = adx > ady ? adx : ady;
  if (steps < 1) steps = 1;

  int sx = (dx > 0) - (dx < 0);
  int sy = (dy > 0) - (dy < 0);

  int accX = 0, accY = 0;
  for (int i = 0; i < steps; i++) {
    accX += adx;
    if (accX >= steps) {
      accX -= steps;
      if (advanceBallUnit(sx, 0)) return;
    }
    accY += ady;
    if (accY >= steps) {
      accY -= steps;
      if (advanceBallUnit(0, sy)) return;
    }
  }
}

// Move the ball exactly one pixel and resolve walls / paddles / goals.
// Returns true when this tick should stop early (wall bounce, paddle hit or
// goal) so the remaining sub-steps don't fight the freshly changed direction.
bool Pong::advanceBallUnit(int dx, int dy) {
  int nx = (int)ballX + dx;
  int ny = (int)ballY + dy;
  bool stop = false;

  // top / bottom wall - only when we are actually moving vertically
  if (dy != 0 && ny <= 0) {
    ny = 0;
    ballSpeedY = -ballSpeedY;
    player->onBorderTouched();
    stop = true;
  } else if (dy != 0 && ny >= 31) {
    ny = 31;
    ballSpeedY = -ballSpeedY;
    player->onBorderTouched();
    stop = true;
  }

  // clamp so the uint8_t position can never wrap around
  if (nx < 0) nx = 0;
  else if (nx > 31) nx = 31;
  if (ny < 0) ny = 0;
  else if (ny > 31) ny = 31;

  ballX = (uint8_t)nx;
  ballY = (uint8_t)ny;

  // paddle collision, evaluated on every pixel of the path
  if (checkBall4Collision(0, posBarLeftY, true)) {
    onPaddleHit();
    return true;
  }
  if (checkBall4Collision(31, posBarRightY, false)) {
    onPaddleHit();
    return true;
  }

  // reached a goal line without being deflected
  if (ballX == 0) {
    ++goalsRight;
    goalDetected = true;
    return true;
  } else if (ballX == 31) {
    ++goalsLeft;
    goalDetected = true;
    return true;
  }

  return stop;
}

void Pong::onPaddleHit() {
  if (rallyHits < 250)
    rallyHits++;
}

// True when the 2x2 ball's vertical span [ballY, ballY+1] overlaps the paddle
// rows [barTopY, barTopY + HEIGHT_BAR - 1]. The old check only looked at the
// ball's top row, so a ball whose lower pixel was flush with the paddle's top
// counted as a goal even though it visually sat on the paddle.
bool Pong::ballOverlapsBar(uint8_t barTopY) {
  int top = (int)barTopY;
  int bottom = top + HEIGHT_BAR - 1;
  return (int)ballY <= bottom && (int)ballY + 1 >= top;
}

/**
The function checks whether the ball collides with the left or right paddle.
If so, the ball's direction of movement is changed and the vertical speed
(ballSpeedY) is adjusted based on the point of impact.

border_x ... x-coordinate of the racket boundary (0 = left, 31 = right)
barTopY  ... y-coordinate of the top edge of the racket
isLeft   ... whether the left (true) or right (false) racket is being tested
*/
bool Pong::checkBall4Collision(uint8_t border_x, uint8_t barTopY, bool isLeft) {

  if (isLeft) {
    if (ballX <= border_x && ballOverlapsBar(barTopY)) {
      player->onHitLeftRacket();
      ballSpeedX = (-1) * ballSpeedX;
      int hitPosition = ballY - barTopY;
      ballSpeedY = hitPosition - HEIGHT_BAR / 2;
      if (ballSpeedY < -2) ballSpeedY = -2;
      if (ballSpeedY > 2) ballSpeedY = 2;
      // A dead-flat return makes the ball bounce horizontally forever, so give
      // a centre hit at least a slight up/down component.
      if (ballSpeedY == 0) ballSpeedY = (random(0, 2) == 0) ? -1 : 1;
      return true;
    }
  } else {
    if (ballX >= border_x && ballOverlapsBar(barTopY)) {
      player->onHitRightRacket();
      ballSpeedX = (-1) * ballSpeedX;
      int hitPosition = ballY - barTopY;
      ballSpeedY = hitPosition - HEIGHT_BAR / 2;
      if (ballSpeedY < -2) ballSpeedY = -2;
      if (ballSpeedY > 2) ballSpeedY = 2;
      // A dead-flat return makes the ball bounce horizontally forever, so give
      // a centre hit at least a slight up/down component.
      if (ballSpeedY == 0) ballSpeedY = (random(0, 2) == 0) ? -1 : 1;
      return true;
    }
  }
  return false;
}

// ------- Drawing ------

void Pong::drawBar(uint8_t y, bool isLeft) {
  uint8_t ymax = 32 - HEIGHT_BAR;
  uint8_t dy = (y > ymax) ? ymax : y;
  uint8_t x = isLeft ? 0 : 31;

  for (int z = dy; z < dy + HEIGHT_BAR; z++) {
    GraphicsMem::screen[x][z][0] = 255;
    GraphicsMem::screen[x][z][1] = 0;
    GraphicsMem::screen[x][z][2] = 0;
  }
}

void Pong::drawBall(uint8_t x, uint8_t y) {
  // 2x2 block so the ball is easy to follow on the 32x32 matrix
  for (int ox = 0; ox < 2; ox++) {
    for (int oy = 0; oy < 2; oy++) {
      int px = x + ox;
      int py = y + oy;
      if (px > 31) px = 31;
      if (py > 31) py = 31;
      GraphicsMem::screen[px][py][0] = 0;
      GraphicsMem::screen[px][py][1] = 255;
      GraphicsMem::screen[px][py][2] = 0;
    }
  }
}

// Dashed centre net for the classic Pong look. Drawn LAST each frame and only
// on pixels the ball / trail have not already claimed, so the centre column
// composites the same way no matter which direction the ball crosses it.
void Pong::drawNet() {
  for (int yy = 0; yy < 32; yy += 4) {
    for (int k = 0; k < 2; k++) {
      int y = yy + k;
      if (y > 31) break;
      if (GraphicsMem::screen[15][y][0] || GraphicsMem::screen[15][y][1] ||
          GraphicsMem::screen[15][y][2])
        continue;  // ball or trail owns this pixel this frame
      GraphicsMem::screen[15][y][0] = 70;
      GraphicsMem::screen[15][y][1] = 70;
      GraphicsMem::screen[15][y][2] = 70;
    }
  }
}

void Pong::clearTrail() {
  trailHead = 0;
  trailCount = 0;
}

void Pong::pushTrail(uint8_t x, uint8_t y) {
  trailHead = (trailHead + 1) % TRAIL_LEN;
  trailX[trailHead] = x;
  trailY[trailHead] = y;
  if (trailCount < TRAIL_LEN)
    trailCount++;
}

// Fading green tail behind the ball (QW4).
void Pong::drawTrail() {
  for (uint8_t age = 1; age < trailCount; age++) {
    uint8_t idx = (trailHead + TRAIL_LEN - age) % TRAIL_LEN;
    int g = 150 - age * 32;
    if (g < 10) g = 10;

    uint8_t tx = trailX[idx];
    uint8_t ty = trailY[idx];
    for (int ox = 0; ox < 2; ox++) {
      for (int oy = 0; oy < 2; oy++) {
        int px = tx + ox;
        int py = ty + oy;
        if (px > 31 || py > 31) continue;
        if (GraphicsMem::screen[px][py][1] < g) {
          GraphicsMem::screen[px][py][0] = 0;
          GraphicsMem::screen[px][py][1] = (uint8_t)g;
          GraphicsMem::screen[px][py][2] = 0;
        }
      }
    }
  }
}

void Pong::drawCountdownDigit(int n) {
  if (n < 1 || n > 3)
    return;

  const uint8_t *g = COUNT_GLYPH[3 - n];  // n == 3 -> first glyph
  const int scale = 4;
  const int originX = 16 - (3 * scale) / 2;
  const int originY = 16 - (5 * scale) / 2;

  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 3; col++) {
      if (!(g[row] & (1 << (2 - col))))
        continue;
      for (int oy = 0; oy < scale; oy++) {
        for (int ox = 0; ox < scale; ox++) {
          int px = originX + col * scale + ox;
          int py = originY + row * scale + oy;
          if (px < 0 || px > 31 || py < 0 || py > 31)
            continue;
          GraphicsMem::screen[px][py][0] = 255;
          GraphicsMem::screen[px][py][1] = 120;
          GraphicsMem::screen[px][py][2] = 0;
        }
      }
    }
  }
}

// Firework palette. v is the current brightness (0..255); index picks the hue.
static void fireColor(uint8_t index, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b) {
  switch (index % 6) {
    case 0:  r = v;       g = v / 5;   b = v / 5;   break;  // red
    case 1:  r = v;       g = v * 2/3; b = 0;       break;  // gold
    case 2:  r = v / 4;   g = v;       b = v / 4;   break;  // green
    case 3:  r = v / 4;   g = v / 2;   b = v;       break;  // blue
    case 4:  r = v;       g = v / 4;   b = v;       break;  // magenta
    default: r = v;       g = v;       b = v;       break;  // white
  }
}

static void putPixelMax(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || x > 31 || y < 0 || y > 31) return;
  if (r > GraphicsMem::screen[x][y][0]) GraphicsMem::screen[x][y][0] = r;
  if (g > GraphicsMem::screen[x][y][1]) GraphicsMem::screen[x][y][1] = g;
  if (b > GraphicsMem::screen[x][y][2]) GraphicsMem::screen[x][y][2] = b;
}

// End-of-match celebration, fully driven by the elapsed time t so it stays
// smooth at any frame rate. Runs in demo mode as well.
//   phase A  (0 .. 800 ms) : a gold wipe sweeps in from the winner's edge
//   phase B  (800 ms .. end): expanding firework rings + winner-edge glow
void Pong::drawMatchOver(unsigned long now) {
  unsigned long t = now - phaseStart;
  bool leftWon = goalsLeft >= winScore;

  // --- phase A: colour wipe -------------------------------------------------
  const unsigned long WIPE_MS = 800;
  if (t < WIPE_MS) {
    int front = (int)(t * 32 / WIPE_MS);
    for (int i = 0; i <= front && i < 32; i++) {
      int x = leftWon ? i : 31 - i;
      int fade = 255 - (front - i) * 12;
      if (fade < 0) fade = 0;
      for (int y = 0; y < 32; y++)
        putPixelMax(x, y, fade, fade * 3 / 4, 0);
    }
    return;
  }

  // --- phase B: fireworks -------------------------------------------------
  unsigned long bt = t - WIPE_MS;
  const int BURST_EVERY = 600;   // ms between launches
  const int BURST_LIFE = 1500;   // ms a burst stays visible

  int newest = (int)(bt / BURST_EVERY);
  for (int burst = newest; burst >= 0; burst--) {
    long age = (long)bt - (long)burst * BURST_EVERY;
    if (age < 0 || age > BURST_LIFE) continue;

    // stable pseudo-random position / colour for this burst
    uint32_t h = (uint32_t)(burst + 1) * 2654435761u;
    int cx = 5 + (int)(h % 22u);
    int cy = 5 + (int)((h >> 7) % 22u);
    uint8_t ci = (uint8_t)((h >> 15) % 6u);

    int r = (int)(age * 14 / BURST_LIFE);
    int bright = 255 - (int)(age * 255 / BURST_LIFE);
    if (bright < 0) bright = 0;

    uint8_t pr, pg, pb;
    fireColor(ci, (uint8_t)bright, pr, pg, pb);

    int rin = (r - 1) * (r - 1);
    int rout = (r + 1) * (r + 1);
    for (int dy = -r - 1; dy <= r + 1; dy++) {
      for (int dx = -r - 1; dx <= r + 1; dx++) {
        int d2 = dx * dx + dy * dy;
        if (d2 < rin || d2 > rout) continue;
        putPixelMax(cx + dx, cy + dy, pr, pg, pb);
      }
    }
    if (age < 110) {
      putPixelMax(cx, cy, 255, 255, 255);
      putPixelMax(cx + 1, cy, 255, 255, 255);
      putPixelMax(cx - 1, cy, 255, 255, 255);
      putPixelMax(cx, cy + 1, 255, 255, 255);
      putPixelMax(cx, cy - 1, 255, 255, 255);
    }
  }

  // steady glow on the winner's goal line so it stays obvious who won
  int glow = 90 + (int)((t / 60) % 120);
  uint8_t edgeX = leftWon ? 0 : 31;
  for (int y = 0; y < 32; y++)
    putPixelMax(edgeX, y, glow, glow, glow);
}

void Pong::clearDisplay() {
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      GraphicsMem::screen[i][j][0] = 0;
      GraphicsMem::screen[i][j][1] = 0;
      GraphicsMem::screen[i][j][2] = 0;
    }
  }
}

// ------- Demo AI ------

void Pong::calcDemoInput() {

  demo_up_left = demo_down_left = 0;
  demo_up_right = demo_down_right = 0;

  // Every so often pick a fresh aim error for each paddle. Aiming a few pixels
  // off centre means the ball is usually returned at an angle (no permanent
  // flat rally) and now and then the paddle is simply too far off to reach it.
  if (++demoTick >= 18) {
    demoTick = 0;
    demoOffsetLeft = random(-3, 4);
    demoOffsetRight = random(-3, 4);
  }

  int targetLeft = (int)ballY - HEIGHT_BAR / 2 + demoOffsetLeft;
  int targetRight = (int)ballY - HEIGHT_BAR / 2 + demoOffsetRight;

  // Small chance per frame to hesitate (skip the move) instead of tracking.
  bool hesitateLeft = random(0, 100) < 12;
  bool hesitateRight = random(0, 100) < 12;

  // +/-1 dead band so the paddle does not jitter around the target.
  if (!hesitateLeft) {
    if (posBarLeftY < targetLeft - 1) demo_down_left = 1;
    else if (posBarLeftY > targetLeft + 1) demo_up_left = 1;
  }
  if (!hesitateRight) {
    if (posBarRightY < targetRight - 1) demo_down_right = 1;
    else if (posBarRightY > targetRight + 1) demo_up_right = 1;
  }
}
