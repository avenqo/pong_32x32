#include "SdCard.h"
#include "log.h"
#include <SD.h>

SdCard::SdCard() {
  logg = new Log("SD");
  currentIndex=0;
  if (!SD.begin()) {
    while (1) {
      Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
      delay(1000);
    }
  }
}

SdCard::~SdCard() {
  
}


char* SdCard::getNextPng() {
  
  // Beim ersten Aufruf: Anzahl der PNG-Dateien zählen
  if (firstCall) {
    firstCall = false;
    root = SD.open(PNG_DIR);
    if (!root) {
      return nullptr; // Verzeichnis konnte nicht geöffnet werden
    }

    // Anzahl der PNG-Dateien zählen
    File entry;
    while (true) {
      entry = root.openNextFile();
      if (!entry) break;
      if (!entry.isDirectory() && strstr(entry.name(), ".png") != nullptr) {
        fileCount++;
      }
      entry.close();
    }
    root.rewindDirectory();
    root.close();
  }

  // Verzeichnis erneut öffnen und zur aktuellen Position springen
  root = SD.open(PNG_DIR);
  if (!root) {
    return nullptr;
  }

  File entry;
  int currentPos = 0;
  char* result = nullptr;

  // Zur nächsten PNG-Datei springen
  while (true) {
    entry = root.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory() && strstr(entry.name(), ".png") != nullptr) {
      if (currentPos == currentIndex) {
        // Vollständigen Pfad konstruieren
        result = new char[strlen(PNG_DIR) + strlen(entry.name()) + 2];
        sprintf(result, "%s/%s", PNG_DIR, entry.name());
        break;
      }
      currentPos++;
    }
    entry.close();
  }

  root.close();

  // Index für den nächsten Aufruf aktualisieren
  if (fileCount > 0) {
    currentIndex = (currentIndex + 1) % fileCount;
  }

  return result;
}
