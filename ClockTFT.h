#pragma once

#include <Adafruit_GC9A01A.h>

class Pixel {
  public:
    int8_t x;
    int8_t y;
    int16_t color;
};

class ClockTFT : public Adafruit_GC9A01A {
  public:
    ClockTFT(int8_t cs, int8_t dc, int8_t rst);

    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

    void writePixel(uint16_t x, uint16_t y, uint16_t color);
    void writePixelRaw(uint16_t x, uint16_t y, uint16_t color);
    void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void writeFatLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint8_t width);
    void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color);
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void startWrite();
    void endWrite();
};
