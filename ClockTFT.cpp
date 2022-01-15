#include "ClockTFT.h"
#include "bmp.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

ClockTFT::ClockTFT(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GC9A01A(cs, dc, rst) {}

int pixelCount = 0;
Pixel pixelBuffer[6000];

void ClockTFT::writePixel(uint16_t x, uint16_t y, uint16_t color) {
  if (pixelCount < sizeof(pixelBuffer) / sizeof(Pixel)) {
    if(color == 0x0000) color = readColor(x, y);
    
    pixelBuffer[pixelCount].x = x;
    pixelBuffer[pixelCount].y = y;
    pixelBuffer[pixelCount].color = color;
    pixelCount++;
  } else {
    writePixelRaw(x, y, color);
  }
}

void ClockTFT::writePixelRaw(uint16_t x, uint16_t y, uint16_t color) {
  setAddrWindow(x, y, 1, 1);
  SPI_WRITE16(color);
}

void ClockTFT::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  startWrite();
  writePixel(x0, y0 + r, color);
  writePixel(x0, y0 - r, color);
  writePixel(x0 + r, y0, color);
  writePixel(x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    writePixel(x0 + x, y0 + y, color);
    writePixel(x0 - x, y0 + y, color);
    writePixel(x0 + x, y0 - y, color);
    writePixel(x0 - x, y0 - y, color);
    writePixel(x0 + y, y0 + x, color);
    writePixel(x0 - y, y0 + x, color);
    writePixel(x0 + y, y0 - x, color);
    writePixel(x0 - y, y0 - x, color);
  }
  endWrite();
}

void ClockTFT::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  startWrite();
  writeFastVLine(x0, y0 - r, 2 * r + 1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
  endWrite();
}

void ClockTFT::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  startWrite();
  writeLine(x0, y0, x1, y1, color);
  endWrite();
}

void ClockTFT::writeFatLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint8_t width) {
  for (int offsetX = -width; offsetX < width; offsetX++) {
    for (int offsetY = -width; offsetY < width; offsetY++) {
      writeLine(x0 + offsetX, y0 + offsetY, x1 + offsetX, y1 + offsetY, color);
    }
  }
}

void ClockTFT::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      writePixel(y0, x0, color);
    } else {
      writePixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void ClockTFT::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;
  int16_t px = x;
  int16_t py = y;

  delta++; // Avoid some +1's in the loop

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    // These checks avoid double-drawing certain lines, important
    // for the SSD1306 library which has an INVERT drawing mode.
    if (x < (y + 1)) {
      if (corners & 1)
        writeFastVLine(x0 + x, y0 - y, 2 * y + delta, color);
      if (corners & 2)
        writeFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
    }
    if (y != py) {
      if (corners & 1)
        writeFastVLine(x0 + py, y0 - px, 2 * px + delta, color);
      if (corners & 2)
        writeFastVLine(x0 - py, y0 - px, 2 * px + delta, color);
      py = y;
    }
    px = x;
  }
}

void ClockTFT::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  writeLine(x, y, x, y + h - 1, color);
}

void ClockTFT::startWrite() {
  pixelCount = 0;
  Adafruit_GC9A01A::startWrite();
}

void ClockTFT::endWrite() {
  for (int i = 0; i < pixelCount; i++) {
    Pixel pixel = pixelBuffer[i];
    writePixelRaw(pixel.x, pixel.y, pixel.color);
  }
  Adafruit_GC9A01A::endWrite();
  pixelCount = 0;
}
