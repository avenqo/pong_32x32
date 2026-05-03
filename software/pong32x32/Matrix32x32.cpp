#include <sys/_stdint.h>
#define PIN 14

#include "Matrix32x32.h"

Matrix32x32::Matrix32x32(int8_t brightness) {
  // logg = new Log("Matrix32x32");

  //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
  //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
  //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
  //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
  //   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
  matrix = new Adafruit_NeoPixel(LED_CNT, PIN, NEO_GRB + NEO_KHZ800);
  delay(500);
  matrix->begin();
  matrix->setBrightness(brightness);
  matrix->show();

}

Matrix32x32::~Matrix32x32() {
  if (matrix != nullptr) {
    delete matrix;
    matrix = nullptr;
  }
}

void Matrix32x32::setBrightness(int8_t brightness) {
  matrix->setBrightness(brightness);
}

void Matrix32x32::drawMemory() {
  unsigned long t1 = micros();
  unsigned long t2 = t1;
  // logg->info("drawMemory() - start");

  for (int x = 0; x < 32; x++) {
    for (int y = 0; y < 32; y++) {

      uint16_t num = mapXY(x, y);

      on(num, GraphicsMem::screen[x][y][0], GraphicsMem::screen[x][y][1], GraphicsMem::screen[x][y][2]);
      //Serial.print(x); Serial.print(","); Serial.print(y); Serial.print(": ("); Serial.print(GraphicsMem::screen[x][y][0]);Serial.print(",");Serial.print(GraphicsMem::screen[x][y][1]);Serial.print(","); Serial.print(GraphicsMem::screen[x][y][2]);Serial.println(")");
    }
  }

  t1 = micros() -t1;

  matrix->show();
  t2 = micros() -t2;
  // 622 micros, 31.621 ms
  // logg->info("drawMemory() - end", t1, t2);
}

/**
  Just setting the memory; without showing.
*/
void Matrix32x32::on(uint16_t num, uint8_t r, uint8_t g, uint8_t b) {
  // turn for 180°
  uint16_t n = (LED_CNT - 1) - num;
  //switch on
  matrix->setPixelColor(n, r, g, b);
  //matrix->show();
}

void Matrix32x32::off(uint16_t num) {
  // logg->info("off()");

  matrix->setPixelColor(num, 0, 0, 0);
  matrix->show();
}

void Matrix32x32::allOff() {
  // logg->info("allOff()");
  matrix->clear();
}

// --------------------
// XY Mapping
// --------------------
// (0, 0) left & top


uint16_t Matrix32x32::mapXY(uint8_t x, uint8_t y) {
  uint8_t a = x / 8;
  uint8_t b = x % 8;
  uint8_t c = y % 8;
  uint8_t d = y / 8;

  return (a * 64) + b + (c * 8) + (d * 256);
}

