#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>
#include "log.h"

class Joystick {
public:
   

    Joystick(int vrxPin, int vryPin, int swPin);
    //void onMovement(void (*callback)(byte));
    void onButtonPress(void (*callback)(byte));
    void update();

     // movement strength (0..4)) 
    uint8_t getMoveValue_Left();
    uint8_t getMoveValue_Right();
    uint8_t getMoveValue_Up();
    uint8_t getMoveValue_Down();
    
private:
    Log* logg;
    int vrxPin, vryPin, swPin;
    int switchState;
    int lastSwitchState;
    byte lastDirection;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

    //void (*movementCallback)(byte);
    void (*buttonCallback)(byte);
    byte getDirection(int x, int y);
};

#endif // JOYSTICK_H