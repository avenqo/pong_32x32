#include <LiquidCrystal_I2C.h>
#include <Encoder.h>
#include <LedControl.h>
#include "RotaryEncoder.h"
#include "Joystick.h"
#include "LCDDisplay.h"
#include "MAX7219Display.h"
#include "SdCard.h"
#include "Matrix32x32.h"
#include "pong.h"
#include "DFPlayer.h"


// Pins
const int encClkPin = 25, encDtPin = 16, encSwPin = 17;
const int maxClkPin = 18, maxDinPin = 23, maxCsPin = 15;
const int joy1VrxPin = 35, joy1VryPin = 34, joy1SwPin = 27;
const int joy2VrxPin = 32, joy2VryPin = 33, joy2SwPin = 26;

// Instanzen
RotaryEncoder* encoder;
Joystick* joy1;
Joystick* joy2;
LCDDisplay* lcd;  // or 0x3F
MAX7219Display* maxDisplay;
SdCard* sdcard;
Log* logg;
Matrix32x32* matrix;
Pong* pong;
DFPlayer* player;

// --- System State ---
enum StateItem { START,
                 DEMO,
                 GAME };
StateItem systemState = START;
const char* stateStrings[] = { "START", "DEMO", "GAME" };

// --- Menu Items ---
enum MenuItem { SPEED,
                VOLUME,
                BRIGHTNESS,
                COLOR };
MenuItem currentMenu = SPEED;
MenuSetup menu;


// --- Loop Variables ---

int MAX = 32;
int x = 0;
int y = 0;
int num = 0;

unsigned int loopCntMillis = millis();
unsigned int loopTimeLimit = menu.speedValue;
StateItem lastSystemState = START;

// ---- Encoder & Menu Selection----

void onEncoderRotation(int direction) {
  logg->info("onEncoderRotation()");
  //static int position = 0;
  //position += direction;
  //lcd->showEncoder(position, direction > 0 ? "Rechts" : "Links ");

  switch (currentMenu) {
    case SPEED:
      // limit the value between 0 and 100
      menu.speedValue = constrain(menu.speedValue + direction, 0, 100);
      loopTimeLimit = menu.speedValue;
      player->onHitRightRacket();
      break;
    case VOLUME:
      menu.volumeValue = constrain(menu.volumeValue + direction, 0, 30);
      player->setVolume( menu.volumeValue);
      delay(300); // without delay -> there is the chance to crash
      // just to gear the difference
      player->onHitLeftRacket(); 
      break;
    case BRIGHTNESS:
      menu.brightnessValue = constrain(menu.brightnessValue + direction, 0, 255);

matrix->setBrightness(menu.brightnessValue );
      matrix->allOff();
      matrix->drawMemory();
      player->onHitRightRacket();

      break;
    case COLOR:
      menu.colorValue = constrain(menu.colorValue + direction, 1, 3);
      break;
  }
  updateMenuDisplay();
}


void updateMenuDisplay() {
  char buffer[17];
  lcd->clear();

  switch (currentMenu) {
    case SPEED:
      snprintf(buffer, sizeof(buffer), "Speed: %3u", menu.speedValue);
      lcd->showText(0, buffer);
      break;
    case VOLUME:
      snprintf(buffer, sizeof(buffer), "Volume: %3u", menu.volumeValue);
      lcd->showText(0, buffer);
      break;
    case BRIGHTNESS:
      snprintf(buffer, sizeof(buffer), "Brightness: %3u", menu.brightnessValue);
      lcd->showText(0, buffer);
      break;
    case COLOR:
      snprintf(buffer, sizeof(buffer), "Color: %s",
               menu.colorValue == 1 ? "Farbe 1" : menu.colorValue == 2 ? "Farbe 2"
                                                                       : "Farbe 3");
      lcd->showText(0, buffer);
      break;
  }
}



void onEncoderButtonPressShort() {
  logg->info("onEncoderButtonPressShort()");

  currentMenu = static_cast<MenuItem>((currentMenu + 1) % 4);
  updateMenuDisplay();
}


void onEncoderButtonPressMedium() {
  logg->info("onEncoderButtonPressMedium()");

  // if in Demo mode -> start Game
  if (systemState == DEMO) {
    switchState(GAME);
    // missing menu handling
  } else if (systemState == GAME) {
  }
}


void onEncoderButtonPressLong() {
  logg->info("onEncoderButtonPressLong()");
  lcd->clear();
  lcd->showText(0, "Restarte Pöng   ");
  delay(500);
  ESP.restart();
}

// --- Joystick Button pressed ---

void onJoystickButtonPress_left(byte buttonByte) {
  logg->info("onJoystickButtonPress_left()");
  static char maxMsg[9];
  snprintf(maxMsg, 9, "A Push");
  maxDisplay->showMessage(maxMsg);
}


void onJoystickButtonPress_right(byte buttonByte) {
  logg->info("onJoystickButtonPress_right()");
  static char maxMsg[9];
  snprintf(maxMsg, 9, "B Push");
  maxDisplay->showMessage(maxMsg);
}

// ------------- Switch State ---------------

void switchState(StateItem newState) {
  boolean allowStateChange = false;

  if (systemState == START) {
    if (newState == DEMO) {
      allowStateChange = true;
    }
  } else if (systemState == DEMO) {
    if (newState == GAME) {
      allowStateChange = true;
    }

  } else if (systemState == GAME) {


  } else {
    maxDisplay->showMessage("E9247");
  }

  if (allowStateChange) {
    systemState = newState;
    lcd->showText(0, "Change State");
    lcd->showText(1, stateStrings[systemState]);
    delay(500);
    logg->info("switch to new state", stateStrings[systemState]);
  }
}

// =============== Setup() ==================

void setup() {

  delay(500);
  Serial.begin(115200);
  delay(500);
  logg = new Log("Main");
  logg->info("=== Start PÖNG 32x32 ===");

  // clear all data
  matrix = new Matrix32x32(menu.brightnessValue );
  matrix->allOff();
  matrix->drawMemory();

  lcd = new LCDDisplay(0x27);  // or 0x3F
  lcd->showText(0, "PöNG 32x32");

  maxDisplay = new MAX7219Display(maxClkPin, maxDinPin, maxCsPin);
  maxDisplay->showMessage("P\xD6NG!"); // PöNG
  
  delay(500);

  encoder = new RotaryEncoder(encClkPin, encDtPin, encSwPin);
  joy1 = new Joystick(joy1VrxPin, joy1VryPin, joy1SwPin);
  joy2 = new Joystick(joy2VrxPin, joy2VryPin, joy2SwPin);

  sdcard = new SdCard();

  player = new DFPlayer();
  player->init();
  player->setVolume(menu.volumeValue);

  pong = new Pong(maxDisplay, player);

  // Callbacks registrieren
  encoder->onRotation(onEncoderRotation);
  encoder->onButtonShortPress(onEncoderButtonPressShort);
  encoder->onButtonMediumPress(onEncoderButtonPressMedium);
  encoder->onButtonLongPress(onEncoderButtonPressLong);
  logg->info("Callbacks for encoder are registered");

  //joy1->onMovement(onJoystickMovement_left);
  joy1->onButtonPress(onJoystickButtonPress_left);
  //joy2->onMovement(onJoystickMovement_right);
  joy2->onButtonPress(onJoystickButtonPress_right);

  

  maxDisplay->showMessage("Ready");

  // at this point we are still in START mode
}

// =============== loop() ==================


void loop() {

  // handle menu adjustments
  encoder->update();

  if (systemState == START) {
    maxDisplay->showMessage("START");
    if ((millis() - loopCntMillis) > 5000) {
      // switch to next status
      loopCntMillis = millis();
      switchState(DEMO);
    }
  } else if (systemState == DEMO) {
    maxDisplay->showMessage("DEMO");
  } else if (systemState == GAME) {

    // State switched? Starte Game!
    if (lastSystemState != systemState) {
      pong->startGame();
    }

    // Update the game
    static unsigned long timer = millis();
    if ((millis() - timer) > loopTimeLimit) {
      timer = millis();

      joy1->update();
      joy2->update();

      // Calculate the number only every second time.
      static boolean boolCalcPong = false;
      pong != pong;
      if (pong) {
        pong->loop(joy1, joy2);
      }

      // here the Matrix is drawn from memory as set by pong->loop()
      matrix->drawMemory();
    }
  } else {
    maxDisplay->showMessage("E9246");
  }

  lastSystemState = systemState;
}