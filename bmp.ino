// File MUST be SCREEN_DIAMETER x SCREEN_DIAMETER pixels in 565 format uncompressed

void drawBMPFromFile(int xOffset, int yOffset, int rotationX, int rotationY, int rotation, boolean erase, char *fileName) {
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    Serial.printf_P(PSTR("drawBMPFromFile: can't open '%s'\n"), fileName);
    return;
  }

  uint32_t imageDataOffset;
  uint16_t width, height, bitsPerPixel, dibSize;

  file.seek(0x0a); file.read((uint8_t *)&imageDataOffset, 4);
  file.seek(0x0e); file.read((uint8_t *)&dibSize, 2);
  file.seek(0x12); file.read((uint8_t *)&width, 2);
  file.seek(0x16); file.read((uint8_t *)&height, 2);
  file.seek(0x1c); file.read((uint8_t *)&bitsPerPixel, 2);

  uint16_t colorTable[2] = {0, 0};
  if (bitsPerPixel == 1) {
    for (uint8_t i = 0; i < 2; i++) {
      file.seek(0x0e + dibSize + i * 4);
      uint32_t entry;
      file.read((uint8_t *)&entry, 4);
      colorTable[i] = (((uint8_t *)&entry)[0] & 0xF8) << 8 |
                      (((uint8_t *)&entry)[1] & 0xFC) << 3 |
                       ((uint8_t *)&entry)[2] >> 3;
    }
  }

  // Row byte width padded to 4-byte boundary
  uint16_t rowBytes = (bitsPerPixel == 1)
    ? ((width + 31) / 32) * 4
    : ((width * 2 + 3) / 4) * 4;

  // 256 bytes covers any 16-bit BMP up to 128px wide and any 1-bit mask on this 240px display
  uint8_t rowBuf[256];
  if (rowBytes > sizeof(rowBuf)) {
    Serial.printf_P(PSTR("drawBMPFromFile: row too wide (%d bytes) in '%s'\n"), rowBytes, fileName);
    file.close();
    return;
  }

  Serial.printf_P(PSTR("drawBMPFromFile: '%s' %dx%d @%dbpp rot=%d erase=%d\n"),
                  fileName, width, height, bitsPerPixel, rotation, erase);

  tft.startWrite();
  file.seek(imageDataOffset);

  for (uint16_t file_row = 0; file_row < height; file_row++) {
    file.read(rowBuf, rowBytes);
    yield();
    uint16_t y = height - 1 - file_row; // BMP rows are stored bottom-to-top

    for (uint16_t x = 0; x < width; x++) {
      uint16_t color;
      if (bitsPerPixel == 1) {
        uint8_t bit = (rowBuf[x / 8] >> (7 - (x % 8))) & 1;
        color = colorTable[bit];
      } else {
        color = *(uint16_t *)(rowBuf + x * 2);
      }

      if (color == 0) continue; // transparent pixel
      if (erase) color = 0;    // triggers readColor() inside writePixel for background restore

      switch (rotation) {
        case 0:
          tft.writePixel(xOffset - rotationX + x, yOffset - rotationY + y, color);
          break;
        case 90:
          tft.writePixel(xOffset - (height - rotationY) + (height - y), yOffset - rotationX + x, color);
          break;
        case 180:
          tft.writePixel(xOffset + rotationX - x, yOffset + rotationY - y, color);
          break;
        case 270:
          tft.writePixel(xOffset + (height - rotationY) - (height - y), yOffset + rotationX - x, color);
          break;
      }
    }
  }

  tft.endWrite();
  file.close();
  printFreeRam();
}

File bitmapFile;
long bitmapOffset;

void randomClockFace() {
  currentFaceNumber = random(1, NUMBER_OF_FACE_FILES + 1);

  readHandPositions("hoursHand.pos", hoursHandPositions);
  readHandPositions("minutesHand.pos", minutesHandPositions);

  char buffer[50];
  snprintf_P(buffer, sizeof(buffer), PSTR("/%d/clockface.bmp"), currentFaceNumber);
  if (initBMP(buffer)) {
    drawBMP(tft);
  }
}

boolean initBMP(char* fileName) {
  bitmapFile = LittleFS.open(fileName, "r");
  if (!bitmapFile) {
    Serial.printf_P(PSTR("initBMP: Can't open '%s'\n"), fileName);
    return false;
  }
  bitmapFile.seek(10);
  bitmapFile.read((uint8_t *)&bitmapOffset, sizeof(bitmapOffset));
  return true;
}

void drawBMP(ClockTFT &tft) {
  Serial.printf_P(PSTR("drawBMP: bitmapOffset=%d\n"), bitmapOffset);

  tft.startWrite();
  tft.setAddrWindow(0, 0, SCREEN_DIAMETER, SCREEN_DIAMETER);

  uint16_t buffer[SCREEN_DIAMETER];
  for (int y = 0; y < SCREEN_DIAMETER; y++) {
    yield();
    long seekPosition = bitmapOffset + (SCREEN_DIAMETER - y - 1) * SCREEN_DIAMETER * 2;
    bitmapFile.seek(seekPosition, SeekSet);
    bitmapFile.read((uint8_t *)buffer, SCREEN_DIAMETER * 2);
    tft.writePixels(buffer, SCREEN_DIAMETER);
  }
  tft.endWrite();
}

uint16_t readColor(int16_t x, int16_t y) {
  long seekPosition = bitmapOffset + ((SCREEN_DIAMETER - y - 1) * SCREEN_DIAMETER + x) * 2;
  bitmapFile.seek(seekPosition, SeekSet);
  uint16_t color;
  bitmapFile.read((uint8_t *)&color, sizeof(color));
  return color;
}
