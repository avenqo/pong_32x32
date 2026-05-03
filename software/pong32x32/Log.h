#ifndef LOG_H
#define LOG_H

#include <Arduino.h>

enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERROR
};

class Log {
public:
    Log(const char* location, LogLevel level = LOG_DEBUG);

    void setLevel(LogLevel level);

    template<typename... Args>
    void debug(Args... args) {
        if (currentLevel <= LOG_DEBUG) {
            printPrefix("DEBUG");
            printAll(args...);
            Serial.println();
        }
    }

    template<typename... Args>
    void info(Args... args) {
        if (currentLevel <= LOG_INFO) {
            printPrefix("INFO ");
            printAll(args...);
            Serial.println();
        }
    }

    template<typename... Args>
    void error(Args... args) {
        if (currentLevel <= LOG_ERROR) {
            printPrefix("ERROR");
            printAll(args...);
            Serial.println();
        }
    }

private:
    const char* location;
    LogLevel currentLevel;

    void printPrefix(const char* level);

    // --- Variadic printing ---
    template<typename T>
    void printAll(T value) {
        Serial.print(value);
    }

    template<typename T, typename... Args>
    void printAll(T first, Args... rest) {
        Serial.print(first);
        Serial.print(" ");
        printAll(rest...);
    }
};

#endif