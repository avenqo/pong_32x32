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

byte Joystick::getDirection(int x, int y) {
  byte direction = 0x00;
  if (x < 1000) {
    direction |= (0b11 << 6);  // LEFT: Bits 7 und 6
  } else if (x > 3000) {
    direction |= (0b11 << 4);  // RIGHT: Bits 5 und 4
  }
  if (y < 1000) {
    direction |= (0b11 << 2);  // UP: Bits 3 und 2
  } else if (y > 3000) {
    direction |= 0b11;  // DOWN: Bits 1 und 0
  }
  // Serial.print("X: ");Serial.print(x); Serial.print(", Y: ");Serial.println(y);
  return direction;
}