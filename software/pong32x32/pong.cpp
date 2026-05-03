#include "pong.h"

// ------- Loop ------

void Pong::loop(Joystick *j_left, Joystick *j_right) {
  if (goalDetected) {
    goalDetected = false;
    onGoal();
  }

  clearDisplay();

  // Ballposition berechnen
  if (--ignoreBall == 0) {
    calcBallPosition();
    ignoreBall = 2;
  }

  // Demo-Mode: Paddle -Bewegung automatisch berechnen
  if (demoMode) {
    calcDemoInput();
    // Verwende die berechneten Demo-Werte für die Paddles
    if (demo_up_left > 0) posBarLeftY = decrement(posBarLeftY);
    else if (demo_down_left > 0) posBarLeftY = increment(posBarLeftY);

    if (demo_up_right > 0) posBarRightY = decrement(posBarRightY);
    else if (demo_down_right > 0) posBarRightY = increment(posBarRightY);
  }
  // Normaler Modus: Joystick-Eingaben verwenden
  else {
    uint8_t up_left = j_left->getMoveValue_Up();
    uint8_t down_left = j_left->getMoveValue_Down();
    if (up_left > 0) posBarLeftY = decrement(posBarLeftY);
    else if (down_left > 0) posBarLeftY = increment(posBarLeftY);

    uint8_t up_right = j_right->getMoveValue_Up();
    uint8_t down_right = j_right->getMoveValue_Down();
    if (up_right > 0) posBarRightY = decrement(posBarRightY);
    else if (down_right > 0) posBarRightY = increment(posBarRightY);
  }

  // Paddles und Ball zeichnen
  drawBar(posBarLeftY, true);
  drawBar(posBarRightY, false);
  drawBall(ballX, ballY);

  matrixDisplay->showMessage(goalsLeft, ":", goalsRight);
}
/**
    Delete this
*/
// Calculate Memory depending on Joysticks
void Pong::loop1(Joystick *j_left, Joystick *j_right) {
#ifdef PONG_DEBUG
  Serial.println("--- pong.loop(...) ---");
#endif

  if (goalDetected) {
    goalDetected = false;
    onGoal();
  }

  clearDisplay();

  // Calc position of ball; this sets goalDetected if found
  if (--ignoreBall == 0) {
    calcBallPosition();
    ignoreBall = 2;
  }

  // calculate position racket LEFT
  uint8_t up_left = j_left->getMoveValue_Up();
  uint8_t down_left = j_left->getMoveValue_Down();

  if (up_left > 0) posBarLeftY = decrement(posBarLeftY);
  else if (down_left > 0) posBarLeftY = increment(posBarLeftY);

  // calculate position racket RIGHT
  uint8_t up_right = j_right->getMoveValue_Up();
  uint8_t down_right = j_right->getMoveValue_Down();
  if (up_right > 0) posBarRightY = decrement(posBarRightY);
  else if (down_right > 0) posBarRightY = increment(posBarRightY);

#ifdef PONG_DEBUG1
  Serial.print("pong.loop -> (down, up) -> LEFT (");
  Serial.print(down_left);
  Serial.print(",");
  Serial.print(up_left);
  Serial.print(") ; RIGHT (");
  Serial.print(down_right);
  Serial.print(",");
  Serial.print(up_right);
  Serial.println(")");
#endif

  // set memory
  drawBar(posBarLeftY, true);
  drawBar(posBarRightY, false);
  drawBall(ballX, ballY);

  matrixDisplay->showMessage(goalsLeft, ":", goalsRight);
}

uint8_t Pong::increment(uint8_t i) {
#ifdef PONG_DEBUG
  Serial.print("pong.increment(");
  Serial.print(i);
  Serial.println(")");
#endif

  uint8_t limit = 32 - HEIGHT_BAR;
  if (i >= limit)
    return limit;
  return ++i;
}

// ------- Game Start ------

void Pong::startGame() {
#ifdef PONG_DEBUG
  Serial.println("pong.startGame()");
#endif
  matrixDisplay->showMessage("START GAME");
  player->startGame();

  goalsLeft = 0;
  goalsRight = 0;
  roundCounter = 0;
  delay(5000);
  startRound();
}  // start button pressed

// ------- Game Round ------

void Pong::startRound() {
#ifdef PONG_DEBUG
  Serial.println("pong.startRound()");
#endif

  goalDetected = false;
  if (demoMode) {
    matrixDisplay->showMessage("DEMO");
    delay(1000);
    matrixDisplay->showMessage("MODE");
  } else {
    matrixDisplay->showMessage("Starte");
  }
  delay(1000);
  matrixDisplay->showMessage("Runde");
  player->startRound();
  delay(1000);

  // surprise y position & direction, but never start from borders/limits
  ballY = random(1, 31);
  ballSpeedY = random(-2, 3);  // -2 .. 2

  // surprise x direction
  ballSpeedX = random(0, 2) * 2 - 1;  // Creates -1 or 1
  ballX = (ballSpeedX > 0) ? 5 : 25;
  /*
  logg->info("ballSpeedX", ballSpeedX);
  logg->info("ballX", ballX);
logg->info("ballSpeedY", ballSpeedY);
  logg->info("ballY", ballY);
  delay(5000);
*/

  drawBall(ballX, ballY);
  roundCounter++;
}

// ------- Draw Bar ------

void Pong::drawBar(uint8_t y, bool isLeft) {
#ifdef PONG_DEBUG1
  Serial.print("pong.drawBar(");
  Serial.print(y);
  Serial.print(",");
  Serial.print(isLeft ? "LEFT" : "RIGHT");
  Serial.println(")");
#endif
  // Height 4? -> ymax: 28
  uint8_t ymax = 32 - HEIGHT_BAR;  // max y coord
  // limit dy to ymax
  uint8_t dy = (y > ymax) ? ymax : y;

  uint8_t x = isLeft ? 0 : 31;

  for (int z = dy; z < dy + HEIGHT_BAR; z++) {
    GraphicsMem::screen[x][z][0] = 255;
    GraphicsMem::screen[x][z][1] = 0;
    GraphicsMem::screen[x][z][2] = 0;

#ifdef PONG_DEBUG1
    Serial.print("pong.drawBar -> (x,y) -> ()");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);
    Serial.println(")");
#endif
  }
}

// ------- Draw Ball ------

void Pong::drawBall(uint8_t x, uint8_t y) {
#ifdef PONG_DEBUG
  Serial.print("pong.drawBall(");
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.println(")");
#endif

  bool isBorder = (x == 0 || y == 0 || x == 31 || y == 31);

  GraphicsMem::screen[x][y][0] = isBorder ? 0 : 0;
  GraphicsMem::screen[x][y][1] = isBorder ? 0 : 255;
  GraphicsMem::screen[x][y][2] = isBorder ? 255 : 0;
}

// Display zurücksetzen
void Pong::clearDisplay() {

#ifdef PONG_DEBUG1
  Serial.println("pong.clearDisplay");
#endif

  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      GraphicsMem::screen[i][j][0] = 0;
      GraphicsMem::screen[i][j][1] = 0;
      GraphicsMem::screen[i][j][2] = 0;
    }
  }
}


uint8_t Pong::decrement(uint8_t i) {

#ifdef PONG_DEBUG
  Serial.print("pong.decrement(");
  Serial.print(i);
  Serial.println(")");
#endif

  if (i == 0)
    return 0;
  else
    return --i;
}

/**
The function checks whether the ball collides with the left or right paddle. 
If so, the ball's direction of movement is changed and the vertical speed (ballSpeedY) 
is adjusted based on the point of impact.

border_x ... The x-coordinate of the racket boundary (0 for left racket, 31 for right racket)
barTopY  ... The y-coordinate of the top edge of the racket.
isLeft.  ... A Boolean value that indicates whether the left (true) or right (false) racket is being tested.

*/
bool Pong::checkBall4Collision(uint8_t border_x, uint8_t barTopY, bool isLeft) {

  if (isLeft) {
    // The ball must be to the left of or on top of the racket (ballX <= border_x).
    // The ball's y-coordinate must be within the height of the racket (ballY >= barY && ballY < barY + HEIGHT_BAR).
    if (ballX <= border_x && ballY >= barTopY && ballY < barTopY + HEIGHT_BAR) {
      player->onHitLeftRacket();
      ballSpeedX = (-1) * ballSpeedX;  // If a collision is detected, the ball's horizontal speed is reversed.
      // The vertical speed (ballSpeedY) is calculated based on the hit position (hitPosition) on the racket.
      int hitPosition = ballY - barTopY;
      ballSpeedY = hitPosition - HEIGHT_BAR / 2;
      if (ballSpeedY < -2) ballSpeedY = -2;
      if (ballSpeedY > 2) ballSpeedY = 2;
      return true;
    }
  } else {
    // Rechter Schläger: Ball kommt von links (ballX >= 31)
    if (ballX >= border_x && ballY >= barTopY && ballY < barTopY + HEIGHT_BAR) {
      player->onHitRightRacket();
      ballSpeedX = (-1) * ballSpeedX;
      int hitPosition = ballY - barTopY;
      ballSpeedY = hitPosition - HEIGHT_BAR / 2;
      if (ballSpeedY < -2) ballSpeedY = -2;
      if (ballSpeedY > 2) ballSpeedY = 2;
      return true;
    }
  }
  return false;
}


void Pong::onGoal() {
  matrixDisplay->showMessage("TOOOR!");
  player->onGoal();
  drawBall(ballX, ballY);  // draw last position
  delay(5000);

  matrixDisplay->showMessage(goalsLeft, ":", goalsRight);  // Aktualisiere die Anzeige
  startRound();
}

void Pong::calcBallPosition() {

#ifdef PONG_DEBUG
  Serial.print("pong.calcBall() -> ballX: ");
  Serial.print(ballX);
  Serial.print(", ballY: ");
  Serial.print(ballY);
  Serial.print(", ballSpeedX: ");
  Serial.print(ballSpeedX);
  Serial.print(", ballSpeedY: ");
  Serial.println(ballSpeedY);
#endif

  // No collision -> calc next position
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Invert direction collision upper / lower border
  if (ballY == 0 || ballY >= 31) {
    ballSpeedY = -ballSpeedY;
    player->onBorderTouched();
#ifdef PONG_DEBUG
    Serial.println("pong.calcBall() -> Invert direction!");
#endif
  }

  // if there is a collision ->goal

  if (!checkBall4Collision(0, posBarLeftY, true))       // left bar
    if (!checkBall4Collision(31, posBarRightY, false))  // right bar
    {
      // no hit on bar -> Goal!
      if (ballX == 0) {
        ++goalsRight;
        goalDetected = true;
      } else if (ballX == 31) {
        ++goalsLeft;
        goalDetected = true;
      }
    }
}


Pong::Pong(MAX7219Display *m, DFPlayer *p) {

  matrixDisplay = m;
  player = p;

  ballX = 15;
  ballY = 15;

  posBarLeftY = 9;
  posBarRightY = 15;

  ballSpeedX = 1;
  ballSpeedY = 0;

  logg = new Log("Encoder");
  ignoreBall = 2;
}

void Pong::calcDemoInput() {

  // Reset
  demo_up_left = demo_down_left = 0;
  demo_up_right = demo_down_right = 0;

  // Zielposition = Ballhöhe (mit Offset für Mitte des Paddles)
  int targetLeft = ballY - HEIGHT_BAR / 2;
  int targetRight = ballY - HEIGHT_BAR / 2;

  // --- Fehler / "Unperfekt" ---
  // 20% Chance auf falsche Entscheidung
  bool errorLeft = random(0, 100) < 20;
  bool errorRight = random(0, 100) < 20;

  // --- LEFT Paddle ---
  if (!errorLeft) {
    if (posBarLeftY < targetLeft) demo_down_left = 1;
    else if (posBarLeftY > targetLeft) demo_up_left = 1;
  } else {
    // falsche Bewegung
    if (random(0, 2) == 0) demo_up_left = 1;
    else demo_down_left = 1;
  }

  // --- RIGHT Paddle ---
  if (!errorRight) {
    if (posBarRightY < targetRight) demo_down_right = 1;
    else if (posBarRightY > targetRight) demo_up_right = 1;
  } else {
    if (random(0, 2) == 0) demo_up_right = 1;
    else demo_down_right = 1;
  }
}
