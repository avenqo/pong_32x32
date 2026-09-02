#include "Joystick.h"

Joystick::Joystick(int vrxPin, int vryPin, int swPin)
  : vrxPin(vrxPin), vryPin(vryPin), swPin(swPin),
    switchState(HIGH), lastSwitchState(HIGH),
    lastDirection(0x00), lastDebounceTime(0), debounceDelay(50),
    //movementCallback(nullptr), 
    buttonCallback(nullptr) {
  pinMode(swPin, INPUT_PULLUP);
}


uint8_t Joystick::getMoveValue_Left() {
  return (lastDirection >> 6) & 0b11;
}

uint8_t Joystick::getMoveValue_Right() {
  return (lastDirection >> 4) & 0b11;
}

uint8_t Joystick::getMoveValue_Down() {
  return (lastDirection >> 2) & 0b11;
}

uint8_t Joystick::getMoveValue_Up() {
  return lastDirection & 0b11;
}
/*
void Joystick::onMovement(void (*callback)(byte)) {
  movementCallback = callback;
}*/

void Joystick::onButtonPress(void (*callback)(byte)) {
  buttonCallback = callback;
}

void Joystick::update() {
  int xVal = analogRead(vrxPin);
  int yVal = analogRead(vryPin);
  byte direction = getDirection(xVal, yVal);
  if (direction != lastDirection) {
    //movementCallback(direction);
    lastDirection = direction;
  }

  // Taster-Logik
  int reading = digitalRead(swPin);
  if (reading != lastSwitchState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != switchState) {
      switchState = reading;
      if (switchState == LOW && buttonCallback) {
        buttonCallback(0x01);
      }
    }
  }
  lastSwitchState = reading;
}

// ESP32 12-bit ADC: 0..4095, resting near the middle.
#define JOY_CENTER 2048
// Below this deflection nothing happens (same dead zone feel as before).
#define JOY_DEADZONE 900
#define JOY_BAND_1 1500
#define JOY_BAND_2 1950

// Map a raw axis reading to a 0..3 "how hard is it pushed" level.
// 0 = centred, 3 = almost at the rail. Pong uses this as the paddle
// step size, so the stick now controls paddle speed, not just up/down.
static uint8_t axisLevel(int v) {
  int d = v - JOY_CENTER;
  if (d < 0) d = -d;
  if (d < JOY_DEADZONE) return 0;
  if (d < JOY_BAND_1) return 1;
  if (d < JOY_BAND_2) return 2;
  return 3;
}

byte Joystick::getDirection(int x, int y) {
  byte direction = 0x00;

  uint8_t lx = axisLevel(x);
  if (lx > 0) {
    if (x < JOY_CENTER) direction |= (lx << 6);  // LEFT:  bits 7..6
    else                direction |= (lx << 4);  // RIGHT: bits 5..4
  }

  uint8_t ly = axisLevel(y);
  if (ly > 0) {
    if (y < JOY_CENTER) direction |= (ly << 2);  // UP:   bits 3..2
    else                direction |= ly;         // DOWN: bits 1..0
  }

  // Serial.print("X: ");Serial.print(x); Serial.print(", Y: ");Serial.println(y);
  return direction;
}