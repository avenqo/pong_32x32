#ifndef SdCard_H
#define SdCard_H

#include "log.h"
#include <SD.h>

#define PIN_SPI_CS 5  // The ESP32 pin GPIO5
#define MAX_PATH_LENGTH 256

class SdCard {

private:
  // directory for PNG files 
  const char* PNG_DIR = "/PNG";


  Log* logg;
  int currentIndex;
  File root;
  bool firstCall = true;
  int fileCount = 0;

  

public:
  // Requires that Serial is alread
  SdCard();
  ~SdCard();

  //void init();
  char* getNextPng();
};
#endif
