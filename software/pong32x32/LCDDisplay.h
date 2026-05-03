#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

class LCDDisplay {
public:
    LCDDisplay(uint8_t address);

    void clear();
    void showText(int pos, const char* txt);
    
    void showEncoder(int pos, const char* dir);
    void showJoystick(const char* name, const char* dir);
    void showButtonPressed(const char* name);

private:
    LiquidCrystal_PCF8574 lcd;
};

#endif // LCD_DISPLAY_H