#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

// in ms; the lower the faster
#define LOOP_TIME_INITIAL 50
// of matrix
#define MATRIX_BRIGHTNESS 16
// of matrix
#define VOLUME_VAL 10

class GraphicsMem {

public:
  static uint8_t screen[32][32][3];
};


class MenuSetup {
public:
  
  unsigned int speedValue = LOOP_TIME_INITIAL;
  uint8_t volumeValue = VOLUME_VAL;
  uint8_t brightnessValue = MATRIX_BRIGHTNESS;
  byte colorValue = 1;
};
#endif