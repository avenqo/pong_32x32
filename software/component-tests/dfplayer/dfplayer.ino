/**
  This code tries to play all content of the SD card sequentially
  and provides detailed diagnostic info.
*/
#include "DFRobotDFPlayerMini.h"
#include <Arduino.h>


HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;


void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

// -------------------- Setup ---------------------------

void setup() {

  mySoftwareSerial.begin(9600, SERIAL_8N1, 4, 13);
  Serial.begin(115200);

  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0);  // Code to compatible with ESP8266 watch dog.
    }
  }

  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(10);  // Lautstärke auf 25 (max. 30)
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  Serial.print("readFileCounts(): ");
  Serial.println(myDFPlayer.readFileCounts(DFPLAYER_DEVICE_SD));
  Serial.println("--- Setup finished ---");
}

// -------------------- Loop ---------------------------

// play all numbers being part of a[]
void playArray() {
  static int a[] = { 1, 5, 6, 9, 13, 14, 15, 16, 18, 20, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,42,43,44,45,46,47,48,49,50 };
  static int z = 0;
  static int size = sizeof(a) / sizeof(a[0]);

  Serial.print("playArray() - Play Num: ");
  Serial.println(a[z]);

  // Play number
  myDFPlayer.play(a[z]);

  Serial.print("   a[");
  Serial.print(z);
  Serial.print("]=");
  Serial.println(a[z]);
  if (++z >= size) z = 0;
}

// Plays next and shows the number; you can use this to determine the available numbers
void playNext() {

  // Play next
  myDFPlayer.next();

  Serial.print("playNext() - Play number: ");
  Serial.println(myDFPlayer.readCurrentFileNumber());
}


void loop() {

  static unsigned long timer = millis();

  if (millis() - timer > 3000) {
    timer = millis();

    // playNext();
    playArray();
  }
}