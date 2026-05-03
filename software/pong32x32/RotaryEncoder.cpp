#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(int clkPin, int dtPin, int swPin)
  : encoder(clkPin, dtPin), swPin(swPin) {
  pinMode(swPin, INPUT_PULLUP);
  logg = new Log("Encoder");
}

void RotaryEncoder::onRotation(void (*callback)(int)) {
  rotationCallback = callback;
}

void RotaryEncoder::onButtonShortPress(void (*callback)()) {
  buttonCallbackShort = callback;
}

void RotaryEncoder::onButtonMediumPress(void (*callback)()) {
  buttonCallbackMedium = callback;
}

void RotaryEncoder::onButtonLongPress(void (*callback)()) {
  buttonCallbackLong = callback;
}


void RotaryEncoder::update() {
  long newPosition = encoder.read() / 4; // some simple deboucing
  if (newPosition != oldPosition) {
    int direction = (newPosition > oldPosition) ? -1 : 1;
    oldPosition = newPosition;
    if (rotationCallback) {
      rotationCallback(direction);
    }
  }

  // fetch SW (with debouncing)
  int currentSwState = digitalRead(swPin);

  // Debounce-Logik
  if (currentSwState != lastSwitchState) {
    lastDebounceTime = millis();
  }

  // if State remains stable (afterh Debounce)
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Wenn der Zustand sich geändert hat
    if (currentSwState != lastSwState) {
      lastSwState = currentSwState;

      // Button pushed (HIGH->LOW)
      if (lastSwState == LOW) {
        pressStartTime = millis(); // Zeitpunkt des Drückens speichern
      }
      // Button released(LOW->HIGH)
      else {
        unsigned long pressDuration = millis() - pressStartTime;

        // Callbacks je nach Drückdauer aufrufen
        if (pressDuration >= LONG_DURATION && buttonCallbackLong) {
          buttonCallbackLong();
        }
        else if (pressDuration >= MEDIUM_DURATION && buttonCallbackMedium) {
          buttonCallbackMedium();
        }
        else if (pressDuration >= SHORT_DURATION && buttonCallbackShort) {
          buttonCallbackShort();
        }
      }
    }
  }
  lastSwitchState = currentSwState;
}