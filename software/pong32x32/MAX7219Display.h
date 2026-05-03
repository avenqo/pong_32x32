#ifndef MAX7219_DISPLAY_H
#define MAX7219_DISPLAY_H

#include "MD_Parola.h"
#include "MD_MAX72xx.h"
#include "SPI.h"


class MAX7219Display {
public:
  MAX7219Display(int clkPin, int dinPin, int csPin);
  ~MAX7219Display();


  void showMessage(const char* msg);
  void showMessage(int b1, const char* msg, int b2);

private:
  MD_Parola* disp;
};

#endif  // MAX7219_DISPLAY_H