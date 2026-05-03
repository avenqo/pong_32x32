#include "LCDDisplay.h"

LCDDisplay::LCDDisplay(uint8_t address)
    : lcd(address) {
    Wire.begin();
    lcd.begin(16, 2); // 16 Spalten, 2 Zeilen
    lcd.setBacklight(255);
    lcd.clear();
    lcd.print("Initialisierung...");
    delay(1000);
    lcd.clear();
}

void LCDDisplay::clear(){
    lcd.clear();
}


void LCDDisplay::showText(int row, const char* txt){
    lcd.setCursor(0, row);
    lcd.print(txt);
}

void LCDDisplay::showEncoder(int pos, const char* dir) {
    lcd.setCursor(0, 0);
    lcd.print("Enc: ");
    lcd.print(pos);
    lcd.print(" ");
    lcd.print(dir);
}

void LCDDisplay::showJoystick(const char* name, const char* dir) {
    lcd.setCursor(0, 1);
    lcd.print(name);
    lcd.print(": ");
    lcd.print(dir);
}

void LCDDisplay::showButtonPressed(const char* name) {
    lcd.clear();
    lcd.print(name);
    lcd.print(" gedrueckt!");
    delay(500);
    lcd.clear();
}