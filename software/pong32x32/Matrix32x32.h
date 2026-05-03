#ifndef MATRIX32X32_H
#define MATRIX32X32_H

#define MATRIX_NUMBER 16
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Global.h"
//#include "Log.h"

class Matrix32x32 {

public:
  Matrix32x32(int8_t brightness);
  ~Matrix32x32();

  void setBrightness(int8_t brightness);
  void on(uint16_t num, uint8_t r, uint8_t g, uint8_t b);
  void off(uint16_t num);

  void allOff();
  uint16_t mapXY(uint8_t x, uint8_t y);

  void drawMemory();

  // x, y rgb
  //void showScreen(uint8_t screen[32][32][3]);
  //void setScreen(uint8_t (&screenRef)[32][32][3]); 

private:

  const int LED_CNT = MATRIX_NUMBER * MATRIX_WIDTH * MATRIX_HEIGHT;

  Adafruit_NeoPixel* matrix;
 // Log* logg;
};

#endif  // MAX7219_DISPLAY_H