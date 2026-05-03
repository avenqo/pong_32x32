#include "Global.h"

uint8_t GraphicsMem::screen[32][32][3];

const char* stateStrings[] = { "START", "DEMO", "GAME" };
StateItem systemState = START;