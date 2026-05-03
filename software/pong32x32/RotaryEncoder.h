#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <Arduino.h>
#include <Encoder.h>
#include "log.h"

#define LONG_DURATION 6000
#define MEDIUM_DURATION 1000
#define SHORT_DURATION 50

class RotaryEncoder{
public:
    RotaryEncoder(int clkPin, int dtPin, int swPin);
    void onRotation(void (*callback)(int));

    // 50ms ... < 1 sec
    void onButtonShortPress(void (*callback)());
    // > 1 ... < 3 sec
    void onButtonMediumPress(void (*callback)());
    // > 6 sec
    void onButtonLongPress(void (*callback)());

    void update();

private:
    Log* logg;
  
    Encoder encoder;
    int swPin;
    long oldPosition = -999;
    int lastSwState = HIGH;
    int lastSwitchState = HIGH;
    
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = SHORT_DURATION;
    unsigned long pressStartTime; // Speichert den Zeitpunkt, wann der Button gedrückt wurde

    void (*rotationCallback)(int) = nullptr;

    void (*buttonCallbackShort)() = nullptr;
    void (*buttonCallbackMedium)() = nullptr;
    void (*buttonCallbackLong)() = nullptr;
};

#endif // ROTARY_ENCODER_H