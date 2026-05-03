#include "Log.h"

#define ANSI_RESET   "\033[0m"
#define ANSI_RED     "\033[31m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"

Log::Log(const char* location, LogLevel level)
    : location(location), currentLevel(level) {
}

void Log::setLevel(LogLevel level) {
    currentLevel = level;
}

void Log::printPrefix(const char* level) {

  const char* color;

    if (strcmp(level, "ERROR") == 0) {
        color = ANSI_RED;
    } else if (strcmp(level, "INFO ") == 0) {
        color = ANSI_GREEN;
    } else {
        color = ANSI_YELLOW;
    }

    Serial.print(color);

    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");

    Serial.print(level);
    Serial.print(" [");
    Serial.print(location);
    Serial.print("]: ");

    Serial.print(ANSI_RESET);
}