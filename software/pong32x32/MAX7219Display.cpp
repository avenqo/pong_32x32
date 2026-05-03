#include "MAX7219Display.h"

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 15


MAX7219Display::MAX7219Display(int clkPin, int dinPin, int csPin) {
  disp = new MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

  disp->begin();
  disp->setIntensity(4);
  disp->displayClear();
  disp->setInvert(false);
  disp->setTextAlignment(PA_LEFT);
  
}

 void MAX7219Display::showMessage(int b1, const char* msg, int b2){
  static char buffer[10];
  char value1Str[4];
  char value2Str[4];
  itoa(b1, value1Str, 10);
  itoa(b2, value2Str, 10);

  strcpy(buffer, value1Str);  // Byte 1
  strcat(buffer, msg);        // const char*
  strcat(buffer, value2Str);  // Byte 2

  showMessage(buffer);
}




void MAX7219Display::showMessage(const char* msg) {
  disp->print(msg);
}

MAX7219Display::~MAX7219Display() {
  if (disp != nullptr) {
    delete disp;
    disp = nullptr;
  }
}
