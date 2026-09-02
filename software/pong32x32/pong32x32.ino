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
#include "Global.h"

#define DEBUG_PONG

// Pins
const int encClkPin = 25, encDtPin = 16, encSwPin = 17;
const int maxClkPin = 18, maxDinPin = 23, maxCsPin = 15;
const int joy1VrxPin = 35, joy1VryPin = 34, joy1SwPin = 27;
const int joy2VrxPin = 32, joy2VryPin = 33, joy2SwPin = 26;

// Instanzen
RotaryEncoder* encoder;
Joystick* joy1;
Joystick* joy2;
LCDDisplay* lcd;
MAX7219Display* maxDisplay;
SdCard* sdcard;
Log* logg;
Matrix32x32* matrix;
Pong* pong;
DFPlayer* player;


// --- Menu Items ---
enum MenuItem { MODE, // DEMO, GAME etc.
                SPEED,
                VOLUME,
                BRIGHTNESS,
                WINSCORE,
                ACCEL,
                COLOR };
MenuItem currentMenu = MODE;
MenuSetup menu;


// --- Loop Variables ---

int MAX = 32;
int x = 0;
int y = 0;
int num = 0;

unsigned int loopCntMillis = millis();
// Never let the loop period reach 0 (would make the ball uncontrollable).
unsigned int loopTimeLimit = constrain(menu.speedValue, MIN_LOOP_MS, MAX_LOOP_MS);
StateItem lastSystemState = START;

// ---- Encoder & Menu Selection----

void onEncoderRotation(int direction) {
  logg->info("onEncoderRotation()");
 
  switch (currentMenu) {
    case MODE:
      if (systemState == DEMO) {
         switchState(GAME);
      } else {
         switchState(DEMO);
      }
      
      break;
    case SPEED:
      // speedValue is the loop *delay* in ms, so turning clockwise (direction > 0)
      // must make it smaller = faster. Kept inside [MIN_LOOP_MS, MAX_LOOP_MS].
      menu.speedValue = constrain((int)menu.speedValue - direction * SPEED_STEP,
                                  MIN_LOOP_MS, MAX_LOOP_MS);
      loopTimeLimit = menu.speedValue;
      player->onHitRightRacket();
      break;
    case VOLUME:
      menu.volumeValue = constrain(menu.volumeValue + direction, 0, 30);
      player->setVolume(menu.volumeValue);
      delay(300);  // without delay -> there is the chance to crash
      // just to gear the difference
      player->onHitLeftRacket();
      break;
    case BRIGHTNESS:
      menu.brightnessValue = constrain(menu.brightnessValue + direction, 0, 255);

      matrix->setBrightness(menu.brightnessValue);
      matrix->allOff();
      matrix->drawMemory();
      player->onHitRightRacket();

      break;
    case WINSCORE:
      // cycle through the three allowed targets 3 / 5 / 10
      if (direction > 0)
        menu.winScore = (menu.winScore < 5) ? 5 : 10;
      else
        menu.winScore = (menu.winScore > 5) ? 5 : 3;
      pong->setWinScore(menu.winScore);
      player->onHitLeftRacket();
      break;
    case ACCEL:
      // 0 = off, 1..3 = gentle..strong rally speed-up
      menu.accelValue = constrain(menu.accelValue + direction, 0, 3);
      pong->setAccel(menu.accelValue);
      player->onHitLeftRacket();
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
     case MODE:
      snprintf(buffer, sizeof(buffer), "Mode: %s", stateStrings[systemState]);
      lcd->showText(0, buffer);
      break;
    case SPEED:
      // Show it as what it is: the per-frame delay in ms (smaller = faster).
      snprintf(buffer, sizeof(buffer), "Delay: %3u ms", menu.speedValue);
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
    case WINSCORE:
      snprintf(buffer, sizeof(buffer), "Win at: %2u", menu.winScore);
      lcd->showText(0, buffer);
      break;
    case ACCEL:
      if (menu.accelValue == 0)
        snprintf(buffer, sizeof(buffer), "Accel: OFF");
      else
        snprintf(buffer, sizeof(buffer), "Accel: %u", menu.accelValue);
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

  // 5 menu items (MODE..COLOR) - the old "% 4" made COLOR unreachable.
  currentMenu = static_cast<MenuItem>((currentMenu + 1) % (COLOR + 1));
  updateMenuDisplay();
}


void onEncoderButtonPressMedium() {
  logg->info("onEncoderButtonPressMedium()");

  if (systemState == DEMO) {
    switchState(GAME);
  } else if (systemState == GAME) {
    switchState(DEMO);
  }
}


void onEncoderButtonPressLong() {
  logg->info("onEncoderButtonPressLong()");

  // Two-step confirm: the first long press only arms the restart, a second
  // long press within 4 s actually does it. Prevents an accidental reboot
  // mid-game.
  static unsigned long armedAt = 0;
  if (armedAt != 0 && (millis() - armedAt) < 4000) {
    lcd->clear();
    lcd->showText(0, "Restarte Poeng");
    delay(400);
    ESP.restart();
  } else {
    armedAt = millis();
    lcd->clear();
    lcd->showText(0, "Neustart? Taste");
    lcd->showText(1, "lang halten");
  }
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
    if (newState == DEMO) {
      allowStateChange = true;
    }
  } else {
    maxDisplay->showMessage("E9247");
  }

  if (allowStateChange) {
    systemState = newState;
    lcd->showText(0, "Change State");
    lcd->showText(1, stateStrings[systemState]);
    delay(500);
    logg->info("switch to new state", stateStrings[systemState]);

     if (systemState == DEMO) 
      pong->enableDemoMode();
     else if (newState == GAME) 
      pong->disableDemoMode();
  }
}

// =============== Setup() ==================

void setup() {

  delay(500);
  Serial.begin(115200);
  delay(500);
  logg = new Log("Main");
  logg->info("=== Start PÖNG 32x32 ===");

  // Seed the RNG so ball serves and the demo AI are not identical every boot.
  // (ADC noise on the joystick inputs + micros() at power-on = enough entropy.)
  randomSeed(micros() ^ (analogRead(joy1VrxPin) << 3) ^ (analogRead(joy2VrxPin) << 7));

  // clear all data
  matrix = new Matrix32x32(menu.brightnessValue);
  matrix->allOff();
  matrix->drawMemory();

  lcd = new LCDDisplay(0x27);  // or 0x3F
  lcd->showText(0, "PöNG 32x32");

  maxDisplay = new MAX7219Display(maxClkPin, maxDinPin, maxCsPin);
  maxDisplay->showMessage("P\xD6NG!");  // PöNG

  delay(500);

  encoder = new RotaryEncoder(encClkPin, encDtPin, encSwPin);
  joy1 = new Joystick(joy1VrxPin, joy1VryPin, joy1SwPin);
  joy2 = new Joystick(joy2VrxPin, joy2VryPin, joy2SwPin);

  sdcard = new SdCard();

  player = new DFPlayer();
  player->init();
  player->setVolume(menu.volumeValue);

  pong = new Pong(maxDisplay, player);
  pong->setWinScore(menu.winScore);
  pong->setAccel(menu.accelValue);

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
  pong->enableDemoMode();
}

// =============== loop() ==================

void loop() {

  // --- crash tracing -------------------------------------------------------
  // Prints a heartbeat with the free heap once a second. If the board freezes,
  // the last line pins down when it died; a steadily falling heap points at a
  // leak, a sudden stop with plenty of heap points at power / peripheral lock-up.
  static unsigned long hbTimer = 0;
  if (millis() - hbTimer > 1000) {
    hbTimer = millis();
    Serial.printf("[hb] t=%lu state=%d heap=%u\n",
                  millis(), (int)systemState, ESP.getFreeHeap());
  }

  // handle menu adjustments
  encoder->update();

  // After Switch on keep the Start state running for 5 sec
  if (systemState == START) {
    maxDisplay->showMessage("START");
    if ((millis() - loopCntMillis) > 5000) {
      // switch to next status
      loopCntMillis = millis();
      switchState(DEMO);
    }
  }

  else if ((systemState == GAME) || (systemState == DEMO)) {

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

      if (pong) {
        pong->loop(joy1, joy2);
      }

      // Repaint the matrix from the framebuffer, but never faster than
      // MIN_DRAW_MS: drawMemory() blocks with interrupts off for ~31 ms, so
      // calling it back-to-back starves the system and can freeze the board.
      static unsigned long drawTimer = millis();
      if ((millis() - drawTimer) >= MIN_DRAW_MS) {
        drawTimer = millis();
        matrix->drawMemory();
      }
    }
  } else {
    maxDisplay->showMessage("E9246");
  }

  lastSystemState = systemState;
}